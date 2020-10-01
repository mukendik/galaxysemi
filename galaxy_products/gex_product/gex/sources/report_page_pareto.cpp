/////////////////////////////////////////////////////////////////////////////
// Creates HTML Pareto page.
/////////////////////////////////////////////////////////////////////////////
#include <qregexp.h>
#include <QLabel>

#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "cbinning.h"
#include "gex_constants.h"			// Constants shared in modules
#include "gex_file_in_group.h"
#include "gex_shared.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "report_classes_sorting.h"	// Classes to sort lists.
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"

// cstats.cpp
extern double			ScalingPower(int iPower);


bool CompareCTestCpk(CTest * test1, CTest * test2)
{
    return (test1->GetCurrentLimitItem()->lfCpk < test2->GetCurrentLimitItem()->lfCpk);
}

bool CompareBinning(CBinning* bin1, CBinning* bin2)
{
    return(bin1->ldTotalCount > bin2->ldTotalCount);
}

bool CompareCTestCp(CTest * test1, CTest * test2)
{
    return(test1->GetCurrentLimitItem()->lfCp < test2->GetCurrentLimitItem()->lfCp);
}

bool CompareCTestFailure(CTest * test1, CTest * test2)
{
    if (test1->GetCurrentLimitItem()->ldFailCount == test2->GetCurrentLimitItem()->ldFailCount)
        return test1->lTestNumber < test2->lTestNumber;

    return(test1->GetCurrentLimitItem()->ldFailCount > test2->GetCurrentLimitItem()->ldFailCount);
}

bool CompareFailureSignature(CMprFailureVector *vector1, CMprFailureVector *vector2)
{
    return(vector1->lFailCount > vector2->lFailCount);
}

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_Pareto(BOOL bValidSection)
{
    char	szString[2048];

    // OPTION(S)
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    // Creates the Pareto page & header
    if (strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Pareto Reports ----\n\n");
    }
    else
    if	(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
    {
        // Generating .HTML report file.
        if(	(strOutputFormat=="HTML")
           && m_pReportOptions->strTemplateFile.isEmpty())
        {
            // Open <stdf-filename>/report/pareto.htm
            sprintf(szString,"%s/pages/pareto.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
            hReportFile = fopen(szString,"wt");
            if(hReportFile == NULL)
                return GS::StdLib::Stdf::ReportFile;
        }
        WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black

        // Title + bookmark: "Pareto reports"
        if(	(strOutputFormat=="HTML")
           && m_pReportOptions->strTemplateFile.isEmpty())
        {
            WriteHtmlSectionTitle(hReportFile,"all_pareto",m_pReportOptions->GetOption("pareto", "section_name").toString());
        }

        if(bValidSection == false)
        {
            fprintf(hReportFile,"<br>\n");
            fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No Pareto data available !<br>\n",iHthmNormalFontSize);
        }
        else
        {
            // Standard HTML page: create short list of hyperlinks to next sections of paretos.
            if (strOutputFormat=="HTML")
            {
                fprintf(hReportFile,"<p align=\"left\">The Pareto tables include reports for:</p>\n");
                fprintf(hReportFile,"<blockquote>\n");
                // OPTIONS
                QString strParetoSectionOptions = (m_pReportOptions->GetOption(QString("pareto"), QString("section"))).toString();
                QStringList qslParetoSectionOptionList = strParetoSectionOptions.split(QString("|"));

                if(qslParetoSectionOptionList.contains(QString("cp")))
                    fprintf(hReportFile,
                      "<p align=\"left\">"
                       "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\">"
                            " <a href=\"#Pareto of Tests Cp\">Tests Cp</a><br>\n");
                if(qslParetoSectionOptionList.contains(QString("cpk")))
                    fprintf(hReportFile,
                      "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\">"
                            " <a href=\"#Pareto of Tests Cpk\">Tests Cpk</a><br>\n");
                if(qslParetoSectionOptionList.contains(QString("failures")))
                    fprintf(hReportFile,
                        "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\">"
                        " <a href=\"#Pareto of Tests failures\">Tests failures</a><br>\n");
                if(qslParetoSectionOptionList.contains(QString("failure_signature")))
                    fprintf(hReportFile,
                        "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\">"
                        " <a href=\"#Pareto of Failure Signatures\">Failure Signatures</a><br>\n");
                if(qslParetoSectionOptionList.contains(QString("soft_bin")))
                    fprintf(hReportFile,
                        "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\">"
                        " <a href=\"#Pareto of Software\">Software Bins</a><br>\n");
                if(qslParetoSectionOptionList.contains(QString("hard_bin")))
                    fprintf(hReportFile,
                        "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\">"
                        " <a href=\"#Pareto of Hardware\">Hardware Bins</a></p>\n");
                fprintf(hReportFile,"</blockquote>\n");
            }
        }
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_Pareto(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else
    if (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
    {
        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_PARETO) == true)
            && (strOutputFormat=="HTML")
            )
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");

            // Close report file...unless creating flat HTML file with multiple sections (eg: MyReport)
            if(m_pReportOptions->strTemplateFile.isEmpty())
                CloseReportFile();
        }
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Write group name in report file (ONLY if multi-groups)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteGroupNameLabel(CGexGroupOfFiles *pGroup)
{
    if(m_pReportOptions->iGroups <= 1)
        return;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    QString strGlobalInfoBookmark;
    // Hyperlink to Global info page.
    if (strOutputFormat=="HTML")
        strGlobalInfoBookmark = "global.htm";
    else
        strGlobalInfoBookmark = "#all_globalinfo";

    // Show group name...and hyperlink to global page.
    if	(strOutputFormat=="CSV")
        fprintf(hReportFile,"\nGroup name: %s\n",pGroup->strGroupName.toLatin1().constData());
    else
        fprintf(hReportFile,"<br><align=\"left\"><b>Group name <a href=\"%s\">: %s</a></b><br>\n",strGlobalInfoBookmark.toLatin1().constData(),pGroup->strGroupName.toLatin1().constData());
}

/////////////////////////////////////////////////////////////////////////////
// Prepare CPK pareto section
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::PrepareSection_CpkParetoReport(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"Pareto of Tests Cpk\n");
    }
    else
    {
        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,"Pareto of Tests Cpk","Pareto of Tests Cpk");
        WriteHtmlToolBar(0,true,"drill_table=stats");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Creates the .CSV or HTML file, with Test Cpk Pareto report
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateCpkParetoReport(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,CTest	*ptTestList)
{
    CTest	*ptTestCell;
    QList <CTest*> qtCpkList;

    char	szTestName[3*GEX_MAX_STRING];	// keep room for testname+pinmap info

    ptTestCell = ptTestList;

    // Write group name + link to GlobalInfo page (HTML): ONLY if multi-groups mode.
    WriteGroupNameLabel(pGroup);

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        fprintf(hReportFile,"Test,Name,Cpk\n");
    }
    else
    {
        // HTML Report: Open Cpk table
        OpenCpCpkParetoTable("Cpk");
    }

    double lfParetoCpkCutoff = m_pReportOptions->GetOption("pareto","cutoff_cpk").toDouble();
    // Create the list of tests & Cpk
    while(ptTestCell != NULL)
    {
        // Only consider valid entries (tests that have a Cpk).
        // IF Muti-result parametric test, do not show master test record
        if((ptTestCell->GetCurrentLimitItem()->lfCpk > C_NO_CP_CPK) &&
        (ptTestCell->lResultArraySize <= 0))
        {
            // Valid Cpk...check if we have a cut-off limit...(a negative cutoff means we list ALL Cpk results)
            if((lfParetoCpkCutoff < 0) ||
                (ptTestCell->GetCurrentLimitItem()->lfCpk <= lfParetoCpkCutoff))
            {
                qtCpkList.append(ptTestCell);
            }
        }
        ptTestCell = ptTestCell->GetNextTest();
    };

    // Sort list
    qSort(qtCpkList.begin(), qtCpkList.end(), CompareCTestCpk);


    // Read list in Ascending order (Smallest Cpk to Biggest)
    int		iColor;	// Color to draw the Cpk chart : we have 4 classes of colors depending of the Cpk value.
    long	lCpkChartSize;
    float	fPercentage;
    QString	strImage;
    QString	strBookmark;
    int		iLinesInPage=0;	// When creating flat HTML, must insert page break at smart position (before too many lines are written)

    foreach(ptTestCell, qtCpkList)
    {
        // Write page break (ignored if not writing a flat HTML document)
        if(iLinesInPage && (iLinesInPage % iLinesPercentagePerFlatPage == 0)
                && ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
            )
        {
            // close table
            fprintf(hReportFile,"</table>\n");

            // Insert page break
            WritePageBreak();

            // Reopen table
            OpenCpCpkParetoTable("Cpk");
        }

        // Create test name string: include PinmapIndex# if exists
        BuildTestNameString(pFile,ptTestCell,szTestName);

        // Read all tests sorted in Ascending order
        if	(strOutputFormat=="CSV")
        {
            fprintf(hReportFile,"%s,%s,%s\n",
                ptTestCell->szTestLabel,
                szTestName,
                CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
        }
        else
        {
            // Bookmark: are in same page if FLAT HTML page is generated
            if (strOutputFormat=="HTML")
                strBookmark.sprintf("stats%d.htm#StatT",ptTestCell->iHtmlStatsPage);
            else
                strBookmark = "#StatT";	// Test Statistics bookmark header string.

            fprintf(hReportFile,"<tr>\n");
            // Include Test# bookmark if Test statistics enabled
            if(ptTestCell->iHtmlStatsPage>0)
                                fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b><a name=\"T%s\"></a> <a href=\"%s%s\">%s</a></b></td>\n",szFieldColor,ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
            else
                fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,ptTestCell->szTestLabel);
            fprintf(hReportFile,"<td width=\"35%%\"  bgcolor=%s align=\"left\">%s</td>\n",szDataColor,szTestName);
            fprintf(hReportFile,"<td width=\"7%%\"  bgcolor=%s align=\"center\">%s</td>\n",szDataColor,CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCpk));
            // Clamp chart...as Cpk > 40 in meaningless.
            if(ptTestCell->GetCurrentLimitItem()->lfCpk <0)
            {
                lCpkChartSize = 1;
                fPercentage = 0;
            }
            else
            if(ptTestCell->GetCurrentLimitItem()->lfCpk <= 40)
            {
                lCpkChartSize = (int) (ptTestCell->GetCurrentLimitItem()->lfCpk*10);
                fPercentage = lCpkChartSize/4;
            }
            else
            {
                lCpkChartSize = 400;	// Resolution is 1/10
                fPercentage = 200;		// 200% is a fake number used to trigger special display mode in the HTML bar!
            }
            if(lCpkChartSize == 0)
                lCpkChartSize = 1;	// Ensure always a tiny bar is shown !
            // Check where Cpk stands, and define color based on it !
            if(lCpkChartSize < 13)
                iColor = 2;	// RED chart for Cpk < 1.33 : Killers
            else
            if(lCpkChartSize < 30)
                iColor = 0xe;	// Dark-RED/Brown chart for Cpk < 3.00 : Unacceptable
            else
            if(lCpkChartSize < 100)
                iColor = 0xc;	// Light-Blue chart for Cpk in  ]3, 10[
            else
                iColor = 1;	// GREEN if Cpk > 10

            // Build Image to use to draw the percentage bar.
            strImage.sprintf("../images/bar%x.png",iColor);

            // If Cpk really big (> 40), add a nice 'speedy' image!
            if(ptTestCell->lfSigma && ptTestCell->GetCurrentLimitItem()->lfCpkLow < 0 && ptTestCell->GetCurrentLimitItem()->lfCpkLow != C_NO_CP_CPK)
                WriteHtmlPercentageBar(hReportFile,49,szDataColor,iColor,true,strImage,lCpkChartSize,fPercentage, "==> Warning: Process is under the low limit");
            else
            if(ptTestCell->lfSigma && ptTestCell->GetCurrentLimitItem()->lfCpkHigh < 0 && ptTestCell->GetCurrentLimitItem()->lfCpkHigh != C_NO_CP_CPK)
                WriteHtmlPercentageBar(hReportFile,49,szDataColor,iColor,true,strImage,lCpkChartSize,fPercentage, "==> Warning: Process is over the high limit");
            else
            if(ptTestCell->GetCurrentLimitItem()->lfCpk>40)
                WriteHtmlPercentageBar(hReportFile,49,szDataColor,iColor,true,strImage,lCpkChartSize,200);
            else
                WriteHtmlPercentageBar(hReportFile,49,szDataColor,iColor,true,strImage,lCpkChartSize,fPercentage);

            fprintf(hReportFile,"</tr>\n");
        }

        // Keep track of the line count.
        iLinesInPage++;

        // Move to next Test in sorted list.
//        ++ptTestCell/* = qtCpkList.next()*/;
    };

    if	(strOutputFormat=="CSV")
        fprintf(hReportFile,"\n");
    else
    {
        fprintf(hReportFile,"</table>\n");

        // Comment about the cutt-off limit
        if(lfParetoCpkCutoff >= 0)
            fprintf(hReportFile,"Shows all Cpk &#60;= %.2g (Defined in <a href=\"_gex_options.htm\">Options</a> , section 'Pareto/Define Cp cut-off limit')<br>\n",
                lfParetoCpkCutoff);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Prepare CP pareto section
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::PrepareSection_CpParetoReport(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"Pareto of Tests Cp\n");
    }
    else
    {
        // HTML Report
        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,"Pareto of Tests Cp","Pareto of Tests Cp");
        WriteHtmlToolBar(0,true,"drill_table=stats");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Open HTML table for: 'Cp Pareto' or 'Cpk Pareto)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::OpenCpCpkParetoTable(const char * szType)
{
    if(hReportFile == NULL)
        return;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if	(strOutputFormat=="HTML")
        fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
    else
    {
        int iCellSpacing;
        if	(strOutputFormat=="PDF")
            iCellSpacing = 2;
        else
            iCellSpacing = 1;
        WriteHtmlOpenTable(98,iCellSpacing);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
    }
    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b>Test</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"35%%\"  bgcolor=%s align=\"left\"><b>Name</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"7%%\"  bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,szType);
    fprintf(hReportFile,"<td width=\"49%%\"  bgcolor=%s align=\"center\"><b>Test %s Chart</b></td>\n",szDataColor,szType);
    fprintf(hReportFile,"</tr>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Creates the .CSV or HTML file, with Test Cp Pareto report
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateCpParetoReport(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,CTest	*ptTestList)
{
    CTest	*ptTestCell=0;
    QList<CTest*> qtCpList;

    char	szTestName[3*GEX_MAX_STRING];	// keep room for testname+pinmap info

    ptTestCell = ptTestList;

    // Write group name + link to GlobalInfo page (HTML): ONLY if multi-groups mode.
    WriteGroupNameLabel(pGroup);

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if	(strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"Test,Name,Cp\n");
    }
    else
    {
        // HTML Report: Open Cp table
        OpenCpCpkParetoTable("Cp");
    }

    bool cpcok=false;
    double lfParetoCpCutoff=m_pReportOptions->GetOption("pareto", "cutoff_cp").toDouble(&cpcok);
    if (!cpcok)
    {
        GSLOG(SYSLOG_SEV_WARNING," error : cant retrieve option (pareto,cutoff_cp)");
    }

    // Create the list of tests & Cp
    //GSLOG(SYSLOG_SEV_DEBUG, "Creating test list...");
    while(ptTestCell != NULL)
    {
        // Only consider valid entries (tests that have a Cp).
        // IF Muti-result parametric test, do not show master test record
        if((ptTestCell->GetCurrentLimitItem()->lfCp > C_NO_CP_CPK) &&
        (ptTestCell->lResultArraySize <= 0))
        {
            // Valid Cp...check if we have a cut-off limit...(a negative cutoff means we list ALL Cp results)
            if((lfParetoCpCutoff < 0) ||
                (ptTestCell->GetCurrentLimitItem()->lfCp <= lfParetoCpCutoff))
            {
                qtCpList.append(ptTestCell);
            }
        }
        ptTestCell = ptTestCell->GetNextTest();
    };

    // Sort list
    //GSLOG(SYSLOG_SEV_DEBUG, "sorting test list...");
    qSort(qtCpList.begin(), qtCpList.end(), CompareCTestCp);

    // Read list in Ascending order (Smallest Cp to Biggest)
    int		iColor;	// Color to draw the Cp chart : we have 4 classes of colors depending of the Cp value.
    long	lCpChartSize;
    float	fPercentage;
    QString	strImage;
    QString	strBookmark;
    int		iLinesInPage=0;	// When creating flat HTML, must insert page break at smart position (before too many lines are written)

    foreach(ptTestCell, qtCpList/*ptTestCell != NULL*/)
    {
        // Write page break (ignored if not writing a flat HTML document)
        if(iLinesInPage && (iLinesInPage % iLinesPercentagePerFlatPage == 0)
            &&
                ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
            )
        {
            // close table
            fprintf(hReportFile,"</table>\n");

            // Insert page break
            WritePageBreak();

            // Reopen table
            OpenCpCpkParetoTable("Cp");
        }

        // Create test name string: include PinmapIndex# if exists
        BuildTestNameString(pFile,ptTestCell,szTestName);

        // Read all tests sorted in Ascending order
        if (strOutputFormat=="CSV")
        {
            fprintf(hReportFile,"%s,%s,%s\n",
                ptTestCell->szTestLabel,
                szTestName,
                CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
        }
        else
        {
            // Bookmark: are in same page if FLAT HTML page is generated
            if (strOutputFormat=="HTML")
                strBookmark.sprintf("stats%d.htm#StatT",ptTestCell->iHtmlStatsPage);
            else
                strBookmark = "#StatT";	// Test Statistics bookmark header string.

            fprintf(hReportFile,"<tr>\n");
            // Include Test# bookmark if Test statistics enabled
            if(ptTestCell->iHtmlStatsPage>0)
                                fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b><a name=\"T%s\"></a> <a href=\"%s%s\">%s</a></b></td>\n",szFieldColor,ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
            else
                fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,ptTestCell->szTestLabel);
            fprintf(hReportFile,"<td width=\"35%%\"  bgcolor=%s align=\"left\">%s</td>\n",szDataColor,szTestName);
            fprintf(hReportFile,"<td width=\"7%%\"  bgcolor=%s align=\"center\">%s</td>\n",szDataColor,CreateResultStringCpCrCpk(ptTestCell->GetCurrentLimitItem()->lfCp));
            // Clamp chart...as Cp > 40 in meaningless.
            if(ptTestCell->GetCurrentLimitItem()->lfCp <= 40)
            {
                lCpChartSize = (int) (ptTestCell->GetCurrentLimitItem()->lfCp*10);
                fPercentage = lCpChartSize/4;
            }
            else
            {
                lCpChartSize = 400;	// Resolution is 1/10
                fPercentage = 200;		// 200% is a fake number used to trigger special display mode in the HTML bar!
            }
            if(lCpChartSize == 0)
            {
                lCpChartSize = 1;	// Ensure always a tiny bar is shown !
                fPercentage = 0;
            }
            // Check where Cp stands, and define color based on it !
            if(lCpChartSize < 13)
                iColor = 2;	// RED chart for Cp < 1.33 : Killers
            else
            if(lCpChartSize < 30)
                iColor = 0xe;	// Dark-RED/Brown chart for Cp < 3.00 : Unacceptable
            else
            if(lCpChartSize < 100)
                iColor = 0xc;	// Light-Blue chart for Cp in  ]3, 10[
            else
                iColor = 1;	// GREEN if Cp > 10

            // Build Image to use to draw the percentage bar.
            strImage.sprintf("../images/bar%x.png",iColor);

            // If Cp really big (> 40), add a nice 'speedy' image!
            WriteHtmlPercentageBar(hReportFile,49,szDataColor,iColor,true,strImage,lCpChartSize,fPercentage);

            fprintf(hReportFile,"</tr>\n");
        }

        // Keep track of the line count.
        iLinesInPage++;
    };

    if (strOutputFormat=="CSV")
        fprintf(hReportFile,"\n");
    else
    {
        fprintf(hReportFile,"</table>\n");
        // Comment about the cutt-off limit
        if(lfParetoCpCutoff >= 0)
            fprintf(hReportFile,"Shows all Cp &#60;= %.2g (Defined in <a href=\"_gex_options.htm\">Options</a> , section 'Pareto/Define Cp cut-off limit')<br>\n",
                lfParetoCpCutoff);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Prepare Failures pareto section
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::PrepareSection_FailuresParetoReport(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"Pareto of Tests failures\n");
    }
    else
    {
        // HTML Report
        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,"Pareto of Tests failures","Pareto of Tests failures");

        if(!ReportOptions.GetOption("pareto","siteRatio").toString().compare("enabled"))
            WriteHtmlToolBar(0,true,"drill_table=stats","Edit Colors","../images/color_palette.png","#_gex_bin_colors.htm");
        else
            WriteHtmlToolBar(0,true,"drill_table=stats");

    }
}


/////////////////////////////////////////////////////////////////////////////
// Open HTML table for: 'Failures Pareto'
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::OpenFailuresParetoTable(void)
{
    if(hReportFile == NULL)
        return;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // HTML Report
    if (strOutputFormat=="HTML")
        fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
    else
    {
        int iCellSpacing;
        if (strOutputFormat=="PDF")
            iCellSpacing = 2;
        else
            iCellSpacing = 1;
        WriteHtmlOpenTable(98,iCellSpacing);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
    }

    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b>Test</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"32%%\"  bgcolor=%s align=\"left\"><b>Name</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>Failing Bin</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>Failures count</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>Yield Loss</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>Fail contribution</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>Test Fail rate </b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"36%%\"  bgcolor=%s align=\"center\"><b>Failures Chart</b></td>\n",szDataColor);
    fprintf(hReportFile,"</tr>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Creates the .CSV or HTML file, with Test Failure Pareto report
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateFailuresParetoReport(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,CTest *ptTestList)
{
    CTest	*ptTestCell=0;
    QList<CTest*> qtFailuresList;
    long	lTotalFailures=0;
    long	lCumulFailBins=0;
    float	fPercentage, fTotalLotYieldLoss, fTotalTestFailRate, fTotalFailContribution;
    char	szTestName[3*GEX_MAX_STRING];	// keep room for testname+pinmap info

    // Generate Statistics info for each test.
    ptTestCell = ptTestList;

    // Write group name + link to GlobalInfo page (HTML): ONLY if multi-groups mode.
    WriteGroupNameLabel(pGroup);

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"Test,Name,Failing Bin,Failures count,Yield Loss,Fail contribution,Test Fail rate\n");
    }
    else
    {
        // HTML Report: Open Failures table
        OpenFailuresParetoTable();
    }

    // Create the list of tests & Failure count
    while(ptTestCell != NULL)
    {
        // Only consider valid entries (tests that have a Cpk).
        // IF Muti-result parametric test, do not show master test record
        if((ptTestCell->GetCurrentLimitItem()->ldFailCount >0) &&
        (ptTestCell->lResultArraySize <= 0))
        {
          qtFailuresList.append(ptTestCell);
          lTotalFailures += ptTestCell->GetCurrentLimitItem()->ldFailCount;	// Compute total failures (so we can compute %)

        }

        // Next test cell
        ptTestCell = ptTestCell->GetNextTest();
    };

    // Sort list
    qSort(qtFailuresList.begin(), qtFailuresList.end(), CompareCTestFailure);

    // Compute the fails parts
    CBinning * ptBinCell = pGroup->cMergedData.ptMergedHardBinList;

    while(ptBinCell != NULL)
    {
        if(toupper(ptBinCell->cPassFail) == 'F')
            lCumulFailBins+= ptBinCell->ldTotalCount;	// Compute cumul of all FAIL bins

        ptBinCell = ptBinCell->ptNextBin;
    }


    // Read list in Descending order (Largest Failure count to smallest)
    long	lFailReference=0;	// Fail count of first cell in pareto (higher count)
    long	lFailChartSize;
    int		iLinesInPage=0;		// When creating flat HTML, must insert page break at smart position (before too many lines are written)
    int		iTotalFailReported=0;
    long	lTotalPartsTested= gex_max(pGroup->cMergedData.lTotalHardBins,pGroup->cMergedData.lTotalSoftBins);
    QString	strYieldLoss;
    QString strTestFailRate;
    QString strFailContribution;
    QString strFailBin;
    QString	strImage;
    QString	strBookmark;

    // Build Image to use to draw the percentage bar: always red
    strImage= "../images/bar2.png";

    // Read list in Descending order (Biggest Failure count to Smallest)
    fTotalLotYieldLoss		= 0;
    fTotalTestFailRate		= 0;
    fTotalFailContribution	= 0;
    foreach(ptTestCell, qtFailuresList)
    {
        // Write page break (ignored if not writing a flat HTML document)
        if(iLinesInPage && (iLinesInPage % iLinesPercentagePerFlatPage == 0)
                && ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
            )
        {
            // close table
            fprintf(hReportFile,"</table>\n");
            // Insert page break
            WritePageBreak();
            // Reopen table
            OpenFailuresParetoTable();
        }

        // Create test name string: include PinmapIndex# if exists
        BuildTestNameString(pFile,ptTestCell,szTestName);

        // Compute Yield Loss (NbFails(test) / NbExecs(Parts))
        if (lTotalPartsTested)
        {
            fPercentage = (((float)100.0*ptTestCell->GetCurrentLimitItem()->ldFailCount)/(float)(lTotalPartsTested));

            // Yield loss (string)
            strYieldLoss.sprintf("%.1f %%", fPercentage);

            fTotalLotYieldLoss += fPercentage;
        }
        else
            strYieldLoss = "n/a";

        // compute Fail Contribution NbFails(test) / NbFails(parts)
        if(lCumulFailBins)
        {
            // Yield loss level
            fPercentage = (((float)100.0*ptTestCell->GetCurrentLimitItem()->ldFailCount)/(float)(lCumulFailBins));

            // Yield loss (string)
            strFailContribution.sprintf("%.1f %%",fPercentage);

            fTotalFailContribution += fPercentage;
        }
        else
            strFailContribution = "n/a";

        // compute Test Fail rate : NbFails(test) / NbFails(test)
        if(ptTestCell->ldSamplesValidExecs)
        {
            // Test fail rate level
            fPercentage = (((float)100.0*ptTestCell->GetCurrentLimitItem()->ldFailCount)/(float)(ptTestCell->ldSamplesValidExecs));

            // Test fail rate (string)
            strTestFailRate.sprintf("%.1f %%",fPercentage);

            fTotalTestFailRate += fPercentage;
        }
        else
            strTestFailRate = "n/a";

        // Compute SoftBin resulting on this test failing.
        if(ptTestCell->iFailBin < 0)
            strFailBin = "-";
        else
            strFailBin = QString::number(ptTestCell->iFailBin);

        if (strOutputFormat=="CSV")
        {
            fprintf(hReportFile,"%s,%s,%s,%d,%s,%s,%s\n",
                ptTestCell->szTestLabel,
                szTestName,
                strFailBin.toLatin1().constData(),
                ptTestCell->GetCurrentLimitItem()->ldFailCount,
                strYieldLoss.toLatin1().constData(),
                strFailContribution.toLatin1().constData(),
                strTestFailRate.toLatin1().constData());
        }
        else
        {
            // Bookmark: are in same page if FLAT HTML page is generated
            if (strOutputFormat=="HTML")
                strBookmark.sprintf("stats%d.htm#StatT",ptTestCell->iHtmlStatsPage);
            else
                strBookmark = "#StatT";	// Test Statistics bookmark header string.

            fprintf(hReportFile,"<tr>\n");
            if(ptTestCell->iHtmlStatsPage>0)
                                fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b><a name=\"T%s\"></a> <a href=\"%s%s\">%s</a></b></td>\n",szFieldColor,ptTestCell->szTestLabel,strBookmark.toLatin1().constData(),ptTestCell->szTestLabel,ptTestCell->szTestLabel);
            else
                fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,ptTestCell->szTestLabel);
            fprintf(hReportFile,"<td width=\"32%%\"  bgcolor=%s align=\"left\">%s</td>\n",	szDataColor,szTestName);
            fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\">%s</td>\n",	szDataColor,strFailBin.toLatin1().constData());
            fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\">%d</td>\n",	szDataColor,ptTestCell->GetCurrentLimitItem()->ldFailCount);
            fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\">%s</td>\n",	szDataColor,strYieldLoss.toLatin1().constData());
            fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\">%s</td>\n",	szDataColor,strFailContribution.toLatin1().constData());
            fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\">%s</td>\n",	szDataColor,strTestFailRate.toLatin1().constData());

            if(iLinesInPage == 0)
            {
                // This is the first FAIL in the pareto: the highest Fail count.
                lFailReference = ptTestCell->GetCurrentLimitItem()->ldFailCount;
            }
            lFailChartSize = (400*ptTestCell->GetCurrentLimitItem()->ldFailCount)/lFailReference;
            if(lFailChartSize == 0)
                lFailChartSize = 1;	// Ensure always a tiny bar is shown !
            fPercentage = lFailChartSize/4;


            bool lDisplaySiteRatio = false;
            if(!ReportOptions.GetOption("pareto","siteRatio").toString().compare("enabled"))
               lDisplaySiteRatio = true;

            WriteHtmlPercentageBar(hReportFile,39,szDataColor,2,true,strImage,lFailChartSize,fPercentage, "", ptTestCell->GetTotalFailPerSite(), lDisplaySiteRatio);

            fprintf(hReportFile,"</tr>\n");
        }

        // Keep track of the line count.
        iLinesInPage++;

        // Keep track of tatal failures listed
        iTotalFailReported++;

        // Check if reached the maximum number of test failures to list...
        if( (m_pReportOptions->GetOption("pareto", "cutoff_failure").toInt()>0)
           &&
            (iTotalFailReported >= m_pReportOptions->GetOption("pareto", "cutoff_failure").toInt())	//iParetoFailCutoff
                   )
            break;	// Exit now: enough test failures reported
    }

    // Write Cumul of failures
    if (strOutputFormat=="CSV")
    {
        fprintf(hReportFile,
                "-,Cumul. of failures,-,%ld,%.1f %%,%.1f %%,%.1f %%\n",
                lTotalFailures, fTotalLotYieldLoss,
                fTotalFailContribution, fTotalTestFailRate);
    }
    else
    {
        // Ensure % remains <= 100%!
        if(fTotalLotYieldLoss > 100.0)
            fTotalLotYieldLoss = 100.0;
        if(fTotalTestFailRate > 100.0)
            fTotalTestFailRate = 100.0;
        if(fTotalFailContribution > 100.0)
            fTotalFailContribution = 100.0;

        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\">-</td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"32%%\"  bgcolor=%s align=\"left\"><b>Cumul. of failures</b></td>\n",szDataColor);
        fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\">-</td>\n",szDataColor);
        fprintf(hReportFile,
            "<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>%ld</b></td>\n",
            szDataColor, lTotalFailures);
        fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>%.1f %%<b></td>\n",szDataColor,fTotalLotYieldLoss);
        fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>%.1f %%</b></td>\n",szDataColor, fTotalFailContribution);
        fprintf(hReportFile,"<td width=\"5%%\"  bgcolor=%s align=\"center\"><b>%.1f %%</b></td>\n",szDataColor, fTotalTestFailRate);
        fprintf(hReportFile,"<td width=\"39%%\" bgcolor=%s align=\"left\">&nbsp;</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");
    }

    if (strOutputFormat=="CSV")
    {
        fprintf(hReportFile,"\n");
        fprintf(hReportFile, "-- Yield loss: number of failed test executions / number of parts\n");
        fprintf(hReportFile, "-- Fail contribution: number of failed test executions / number of parts failed\n");
        fprintf(hReportFile, "-- Test Fail rate: number of failed test executions / number of test executions\n");
    }
    else
    {
        fprintf(hReportFile,"</table>\n");
        fprintf(hReportFile, "-- Yield loss: number of failed test executions / number of parts<br>");
        fprintf(hReportFile, "-- Fail contribution: number of failed test executions / number of parts failed<br>");
        fprintf(hReportFile, "-- Test Fail rate: number of failed test executions / number of test executions<br>");
    }
}



class CMprFailures
{
public:
    CMprFailures();	// Constructor
    QMap<QString,CMprFailureVector> cVectors;	// List of MPR failing vectors & associated failing count.
    long	lTotalFails;	// Cumul of all failures in Failing vectors
};


/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CMprFailures::CMprFailures()
{
    lTotalFails = 0;
}


/////////////////////////////////////////////////////////////////////////////
// Prepare MPR Failures Patterns pareto section
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::PrepareSection_FunctionalFailureSignaturesParetoReport(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"Pareto of Functional Failure Signatures\n");
    }
    else
    {
        // HTML Report
        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,"Pareto of Failure Signatures","Pareto of Functional Failure Signatures (pins tested in parallel)");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Prepare PTR Failures Patterns pareto section
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::PrepareSection_ParametricFailureSignaturesParetoReport(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"Pareto of Parametric Failure Signatures\n");
    }
    else
    {
        // HTML Report
        // Title + bookmark
        WriteHtmlSectionTitle(hReportFile,"Pareto of Failure Signatures","Pareto of Parametric Failure Signatures (tests failing concurrently)");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Open HTML table for: 'Functional Failure Signatures Pareto'
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::OpenFunctionalFailureSignaturesParetoTable(void)
{
    if(hReportFile == NULL)
        return;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // HTML Report
    if	(strOutputFormat=="HTML")
        fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
    else
    {
        int iCellSpacing;
        if (strOutputFormat=="PDF")
            iCellSpacing = 2;
        else
            iCellSpacing = 1;
        WriteHtmlOpenTable(98,iCellSpacing);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
    }

    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>Fail count</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\"><b>%% of failures</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\"><b>%% of tested</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"70%%\"  bgcolor=%s align=\"left\"><b>Functional Failure signatures ( tested pins failing together )</b></td>\n",szFieldColor);
    fprintf(hReportFile,"</tr>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Open HTML table for: 'Parametric Failure Signatures Pareto'
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::OpenParametricFailureSignaturesParetoTable(void)
{
    if(hReportFile == NULL)
        return;

    fprintf(hReportFile,"<br>\n");

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // HTML Report
    if	(strOutputFormat=="HTML")
        fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
    else
    {
        int iCellSpacing;
        if (strOutputFormat=="PDF")
            iCellSpacing = 2;
        else
            iCellSpacing = 1;
        WriteHtmlOpenTable(98,iCellSpacing);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
    }

    fprintf(hReportFile,"<tr>\n");
    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>Fail count</b></td>\n",szFieldColor);
    fprintf(hReportFile,"<td width=\"90%%\"  bgcolor=%s align=\"left\"><b>Parametric Failure signatures ( Tests failing concurrently )</b></td>\n",szFieldColor);
    fprintf(hReportFile,"</tr>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Creates the .CSV or HTML file, with Test Failure Signatures Pareto report
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateFailureSignaturesParetoReport(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,CTest *ptTestList)
{
    // Check if this Examinator release accepts this report
    // Enable/disable some features...
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
    {
        QString m=ReportOptions.GetOption("messages", "upgrade").toString();
        fprintf(hReportFile, "%s", m.toLatin1().data() );
        return;
    }

    CTest	*ptTestCell;
    CTest	*ptTestCellFirst;
    CTest	*ptTestCellLast;

    QList<CMprFailureVector*>	qtFailureSignatureList;

    // Write group name + link to GlobalInfo page (HTML): ONLY if multi-groups mode.
    WriteGroupNameLabel(pGroup);

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    double lfParetoFailPatternCutoff = m_pReportOptions->GetOption("pareto", "cutoff_signature_failure").toDouble();
    QString MPRMergeMode=m_pReportOptions->GetOption("dataprocessing","multi_parametric_merge_mode").toString();


    // Define label separator used when listing failing tests
    QString strLabelSeparator;
    if (strOutputFormat=="CSV")
        strLabelSeparator = " ; ";
    else
        strLabelSeparator = "<br>";

    // Compute list of failure patterns (same failing test# with different pinmap# ; or starting with same test string)
    CMprFailures	cFailingTests;
    QMap<QString,CMprFailureVector>::Iterator itFailingPattern;
    int	iSampleIndex;
    double	lfResult;
    long	ldSamplesInTest = 0;
    QString	strVectorName;
    QString	strTestNamePrefix;
    bool	bTestsAnalyzed=false;
    bool	bUseName=false;

    QRegExp	cRegExp("[^a-z,A-Z,\\d]");	// Definition of what is a delimiter when parsing test's name prefix string: any character except alpha-numerical.

    // Pointing to first test in list
    ptTestCell = ptTestList;

    if(m_pReportOptions->GetOption("dataprocessing", "fail_count").toString() == "all")
    {
        // Continue-on-fail mode: check all tests failing together (including PTR).

        // compute sample count
        while(ptTestCell != NULL)
        {
            // Only consider parametric tests, see how many samples available.
            if(ptTestCell->m_testResult.count() > 0 && ptTestCell->ldSamplesValidExecs > 0 && ptTestCell->bTestType != 'F')
                ldSamplesInTest = gex_max(ldSamplesInTest,ptTestCell->ldSamplesExecs);

            ptTestCell = ptTestCell->GetNextTest();
        };

        // Identify failing patterns
        for(iSampleIndex = 0; iSampleIndex < ldSamplesInTest; iSampleIndex++)
        {
            // Scan test list, identify failures for this run#
            ptTestCell = ptTestList;

            // Clear vector name (list of tests failing this run# index)
            strVectorName = "";

            while(ptTestCell)
            {
                // Only consider parametric tests
                if(ptTestCell->m_testResult.count() == 0 || ptTestCell->bTestType == 'F')
                    goto next_test_continue_fail;

                // check that this test has enough samples!
                if(ptTestCell->m_testResult.isValidIndex(iSampleIndex))
                {
                    if(ptTestCell->m_testResult.isValidResultAt(iSampleIndex))
                    {
                        // check if this test failed this run index
                        lfResult = ptTestCell->m_testResult.resultAt(iSampleIndex);
                        lfResult *= ScalingPower(ptTestCell->res_scal);

                        // Check if result is failing limits
                        if(ptTestCell->isFailingValue(lfResult, ptTestCell->m_testResult.passFailStatus(iSampleIndex)))
                            strVectorName += ptTestCell->strTestName + " (Test " + QString::number(ptTestCell->lTestNumber) + ")" + strLabelSeparator;
                    }
                }

                // Go to next test
next_test_continue_fail:
                ptTestCell = ptTestCell->GetNextTest();
            };

            // Flags that we have analyzed tests (so that even if no result is detected, we were able to perform the analysis)
            bTestsAnalyzed = true;

            // check if above run index# had a failure
            if(strVectorName.isEmpty() == false)
            {
                // Save failure details (list of test names)
                if(cFailingTests.cVectors.find(strVectorName) == cFailingTests.cVectors.end())
                {
                    cFailingTests.cVectors[strVectorName].strVectorName = strVectorName;
                    cFailingTests.cVectors[strVectorName].lFailCount = 1;
                }
                else
                    cFailingTests.cVectors[strVectorName].lFailCount++;

                // Keep track of total failures detected in all vectors
                cFailingTests.lTotalFails++;
            }
        }

        // Save resulting
        CMprFailureVector *pVector;
        for(itFailingPattern = cFailingTests.cVectors.begin(); itFailingPattern != cFailingTests.cVectors.end(); ++itFailingPattern)
        {
            pVector = new CMprFailureVector();
            *pVector = *itFailingPattern;
            qtFailureSignatureList.append(pVector);
        }

        // Reset temporary vector list
        cFailingTests.cVectors.clear();
    }
    else
    while(ptTestCell != NULL)
    {
        // Only consider parametric tests
        if(ptTestCell->m_testResult.count() == 0 || ptTestCell->bTestType == 'F')
            goto next_test;

        // Extract string prefix
        strTestNamePrefix = ptTestCell->strTestName.section(cRegExp,0,0);

        // If no prefix, then check if this is a MPR in whic case, we'll rather use Pinmap index.
        if(strTestNamePrefix.isEmpty() == false)
            bUseName = true;
        else
        {
            // No prefix available
            bUseName = false;

            // Check if this is a MPR, if not, see next test!
            // Only consider valid entries (Muti-result parametric test)
            if(ptTestCell->lPinmapIndex < 0)
                goto next_test;
        }

        // Pointing to first MPR test: find all MPR tests under same test # (only pinmap# differ)
        ptTestCellLast = ptTestCellFirst = ptTestCell;

        // Find last test matching criteria (same name prefix, or if MPR: same test#
        do
        {
            // Check if this test is still part of the list to scan
            if(bUseName)
            {
                // Check if sharing same prefix
                if(strTestNamePrefix != ptTestCell->strTestName.section(cRegExp,0,0))
                    break;
            }
            else
            {
                // Check if sharing same test#
                if(ptTestCell->lTestNumber != ptTestCellFirst->lTestNumber)
                    break;
            }

            // compute maximum of samples in all tests to compare
            ldSamplesInTest = gex_max(ldSamplesInTest,ptTestCell->ldSamplesExecs);

            // Save this pointer location
            ptTestCellLast = ptTestCell;

            // See if next pointer also belongs to the same test list to scan.
            ptTestCell = ptTestCell->GetNextTest();
        }
        while(ptTestCell!=NULL);

        // If only one test found to match the prefix (or pinmap) , then skip it!
        ptTestCell = ptTestCellLast;
        if(ptTestCellFirst == ptTestCellLast)
            goto next_test;

        // We know the range of tests in this MPR, so analyse its failing patterns
        for(iSampleIndex = 0; iSampleIndex < ldSamplesInTest; iSampleIndex++)
        {
            // Clear vector name (list of tests failing this vector index)
            strVectorName = "";

            // Scan all tests and see which one failed this run index.
            ptTestCell = ptTestCellFirst;

            while(1)
            {
                // check that this Parametric test has samples!
                if( (ptTestCell->m_testResult.isValidIndex(iSampleIndex)) && (ptTestCell->ldSamplesValidExecs > 0) )
                {
                    if(ptTestCell->m_testResult.isValidResultAt(iSampleIndex))
                    {
                        // check if this test failed this run index
                        lfResult = ptTestCell->m_testResult.resultAt(iSampleIndex);

                        // If NaN value, do nothing!
                        lfResult *= ScalingPower(ptTestCell->res_scal);
                        // Check if result is failing limits
                        if(ptTestCell->isFailingValue(lfResult, ptTestCell->m_testResult.passFailStatus(iSampleIndex)))
                            strVectorName += ptTestCell->strTestName + " (Test " + QString::number(ptTestCell->lTestNumber) + ")" + strLabelSeparator;
                    }
                }

                // If we've processed the last test pinmap, exit now.
                if(ptTestCell == ptTestCellLast)
                    break;

                // Go to next test pinamp
                ptTestCell = ptTestCell->GetNextTest();
            };

            // Flags that we have analyzed tests (so that even if no result is detected, we were able to perform the analysis)
            bTestsAnalyzed = true;

            // check if above run index# had a failure
            if(strVectorName.isEmpty() == false)
            {
                // Save failure details (list of test names)
                if(cFailingTests.cVectors.find(strVectorName) == cFailingTests.cVectors.end())
                {
                    cFailingTests.cVectors[strVectorName].strVectorName = strVectorName;
                    cFailingTests.cVectors[strVectorName].lFailCount = 1;
                }
                else
                    cFailingTests.cVectors[strVectorName].lFailCount++;

                // Keep track of total failures detected in all vectors
                cFailingTests.lTotalFails++;
            }
        }

        // Save resulting
        CMprFailureVector *pVector;
        for(itFailingPattern = cFailingTests.cVectors.begin(); itFailingPattern != cFailingTests.cVectors.end(); ++itFailingPattern)
        {
            pVector = new CMprFailureVector();
            *pVector = *itFailingPattern;
            qtFailureSignatureList.append(pVector);
        }

        // Reset temporary vector list
        cFailingTests.cVectors.clear();

        // point to last test processed
        ptTestCell = ptTestCellLast;

        // Move to next test#
        next_test:
            ptTestCell = ptTestCell->GetNextTest();
    };


    float	fPercentageOfFails;
    float	fPercentageOfProduction;
    float	fPercentageReported=0;
    long	lTotalPartsTested= gex_max(pGroup->cMergedData.lTotalHardBins,pGroup->cMergedData.lTotalSoftBins);

    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile, "Total devices tested:,%ld\n\n",
                lTotalPartsTested);
        fprintf(hReportFile,"Total patterns detected:,%d\n\n",qtFailureSignatureList.count());
        fprintf(hReportFile,"Fail count,%% of failures,%% of tested,Failure signatures (tested pins failing together)\n");
    }
    else
    {
        // Display total parts tested
        fprintf(hReportFile, "Total devices tested: <b>%ld</b><br>\n",
                lTotalPartsTested);
        fprintf(hReportFile,"Total patterns detected: <b>%d</b><br>\n",qtFailureSignatureList.count());

        // HTML Report: Open Functional Failures table
        OpenFunctionalFailureSignaturesParetoTable();
    }

    // Sort list + force to delete all objects automatically on exit.
    qSort(qtFailureSignatureList.begin(), qtFailureSignatureList.end(), CompareFailureSignature);

    // Read list in Descending order (Largest Failure count to smallest)
    CMprFailureVector * ptVectorEntry = NULL;

    int		iLinesInPage=0;		// When creating flat HTML, must insert page break at smart position (before too many lines are written)
    long	ldTotalFailCount=0;	// USed to add all pattern failures occurances.

    // Read list in Descending order (Biggest Failure count to Smallest)
    foreach(ptVectorEntry, qtFailureSignatureList)
    {
        // Compute failing percentage
        fPercentageOfFails = (((float)100.0*ptVectorEntry->lFailCount)/(float)(cFailingTests.lTotalFails));
        fPercentageOfProduction = (((float)100.0*ptVectorEntry->lFailCount)/(float)(lTotalPartsTested));

        // Keep track of total failing patterns
        ldTotalFailCount += ptVectorEntry->lFailCount;

        // Check if cut-off limit defined (to only output the first X% of signature failures)
        if(lfParetoFailPatternCutoff == 0 || lfParetoFailPatternCutoff >= fPercentageReported)
        {
            // Keep track of total percentage of signature failures reported.
            fPercentageReported += fPercentageOfFails;

            // Write page break (ignored if not writing a flat HTML document)
            if(iLinesInPage && (iLinesInPage % iLinesPercentagePerFlatPage == 0)
                    && ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
                )
            {
                // close table
                fprintf(hReportFile,"</table>\n");

                // Insert page break
                WritePageBreak();

                // Reopen table
                OpenFailuresParetoTable();
            }

            if (strOutputFormat=="CSV")
            {
                fprintf(hReportFile,"%ld,%.2f %%,%.2f %%,%s\n",
                    ptVectorEntry->lFailCount,
                    fPercentageOfFails,
                    fPercentageOfProduction,
                    ptVectorEntry->strVectorName.toLatin1().constData());
            }
            else
            {
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,
                        "<td width=\"10%%\" bgcolor=%s align=\"center\">%ld</td>\n",
                        szDataColor, ptVectorEntry->lFailCount);
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"right\">%.2f %%</td>\n",szDataColor,fPercentageOfFails);
                fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"right\">%.2f %%</td>\n",szDataColor,fPercentageOfProduction);
                fprintf(hReportFile,"<td width=\"70%%\" bgcolor=%s align=\"left\">%s</td>\n",szDataColor,ptVectorEntry->strVectorName.toLatin1().constData());
                fprintf(hReportFile,"</tr>\n");
            }

            // Keep track of the line count.
            iLinesInPage++;
        }

        // Move to next signature failure item.
    }

    if (strOutputFormat=="CSV")
    {
        // Write total failures detected (if some found)
        if(ldTotalFailCount)
            fprintf(hReportFile,"%ld,100 %%, - , Total failures detected\n",ldTotalFailCount);

        fprintf(hReportFile,"\n");

        // Comment about the cutt-off limit
        if(lfParetoFailPatternCutoff >= 0)
            fprintf(hReportFile,"Shows first %.2g %% of the failure signatures (Defined in Options/'Pareto/Define Failure Signatures cut-off limit')\n",
            lfParetoFailPatternCutoff);

        // If no entry, tell possible root cause.
        if(qtFailureSignatureList.count() == 0)
        {
            if(bUseName)
            {
                // Tests analyzed, but not sharing same prefix.
                if(bTestsAnalyzed)
                    fprintf(hReportFile,"No failure signature detected!\nPossible root cause: pins tested in parallel do not have test names starting with a common prefix string (eg: 'continuity_')\n\n");
            }
            else
            {
                // Check if options was set to merge MPR (inn which case option must be toggled to give valide results)
                if(MPRMergeMode=="merge")
                    fprintf(hReportFile,"No failure signature detected!\nPossible root cause: The 'Options/Data processing/Multi-results...' option is set to 'merge' instead of 'no merge'\n\n");
            }
        }
    }
    else
    {
        // Write total failures detected (if some found)
        if(ldTotalFailCount)
        {
            fprintf(hReportFile,"<tr>\n");
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>%ld</b></td>\n",szDataColor,ldTotalFailCount);
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"right\"><b>100 %%</b></td>\n",szDataColor);
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"right\"><b>-</b></td>\n",szDataColor);
            fprintf(hReportFile,"<td width=\"70%%\" bgcolor=%s align=\"left\"><b>Total failures detected</b></td>\n",szDataColor);
            fprintf(hReportFile,"</tr>\n");
        }

        fprintf(hReportFile,"</table>\n");

        // Comment about the cutt-off limit
        if(lfParetoFailPatternCutoff >= 0)
            fprintf(hReportFile,"Shows first %.2g %% of the failure signatures (Defined in <a href=\"_gex_options.htm\">Options</a>, section 'Pareto/Define Failure Signatures cut-off limit')<br>\n",
            lfParetoFailPatternCutoff);

        // If no entry, tell possible root cause.
        if(qtFailureSignatureList.count() == 0)
        {
            if(bUseName)
            {
                // Tests analyzed, but not sharing same prefix.
                if(bTestsAnalyzed)
                    fprintf(hReportFile,"<b>No Functional failure</b> signature detected!<br>Possible root cause: pins tested in parallel do not have test names starting with a common prefix string (eg: 'continuity_')");
            }
            else
            {
                // Check if options was set to merge MPR (inn which case option must be toggled to give valide results)
                if(MPRMergeMode=="merge")
                    fprintf(hReportFile,"<b>No Functional failure</b> signature detected!<br>Possible root cause: The 'Options/Data processing/Multi-results...' option is set to 'merge' instead of 'no merge'" );
            }
        }
    }

    // Check for Parametric Failure signatures
    PrepareSection_ParametricFailureSignaturesParetoReport();

    int	iTotalRuns;
    int	iTotalFailuresInRun;
    QMap <QString, int> mParametricFailurePatterns;
    int	iMinimumConcurrentFails = m_pReportOptions->GetOption("pareto", "min_fail_in_signature").toInt();

    if (pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST,&ptTestCell,true,false) == 1)
    {
        iTotalRuns = ptTestCell->ldSamplesExecs;

        for(iSampleIndex=0;iSampleIndex< iTotalRuns; iSampleIndex++)
        {
            // Clear variables
            iTotalFailuresInRun=0;
            strVectorName = "";

            // Identify how many concurrent PTR failures in run.
            ptTestCell = ptTestList;
            while(ptTestCell)
            {
                // Only look for Parametric tests (PTR).
                if(ptTestCell->bTestType != 'P')
                    goto next_PTEST;

                // check that this Parametric test has samples!
                if(ptTestCell->m_testResult.isValidIndex(iSampleIndex) && (ptTestCell->ldSamplesValidExecs > 0))
                {
                    // check if this test failed this run index
                    if(ptTestCell->m_testResult.isValidResultAt(iSampleIndex))
                    {
                        // Get result.
                        lfResult = ptTestCell->m_testResult.resultAt(iSampleIndex);
                        lfResult *= ScalingPower(ptTestCell->res_scal);

                        // Check if failing result.
                        if(ptTestCell->isFailingValue(lfResult, ptTestCell->m_testResult.passFailStatus(iSampleIndex)))
                        {
                            // Result is below Low Limit
                            strVectorName += "T" + QString::number(ptTestCell->lTestNumber) + " - " + ptTestCell->strTestName + strLabelSeparator;
                            // Keep track of failures
                            iTotalFailuresInRun++;
                        }
                    }
                }

                // Move to next tesp.
                next_PTEST:;
                ptTestCell = ptTestCell->GetNextTest();
            };
            // Single run analyzed: check if multiple failures matching minimum count (as per user option)
            if(iTotalFailuresInRun >= iMinimumConcurrentFails)
            {
                // Multiple PTR failures: update Qmap array!
                if(mParametricFailurePatterns.find(strVectorName) == mParametricFailurePatterns.end())
                    mParametricFailurePatterns[strVectorName] = 1;
                else
                    mParametricFailurePatterns[strVectorName] = mParametricFailurePatterns[strVectorName] + 1;
            }
        }
        // All Runs analyzed, display list of Patterns detected
        if(mParametricFailurePatterns.count() == 0)
        {
            if (strOutputFormat=="CSV")
            {
                fprintf(hReportFile,"\nNo Parametric failure signature detected!\n");
            }
            else
            {
                fprintf(hReportFile,"<br><b>No Parametric failure</b> signature detected!<br>\n");
            }
        }
        else
        {
            if (strOutputFormat=="CSV")
            {
                // ASCII .CSV Report.
                fprintf(hReportFile, "\nTotal devices tested:,%ld\n\n",
                        lTotalPartsTested);
                fprintf(hReportFile,"Total patterns detected:,%d\n\n",mParametricFailurePatterns.count());
                fprintf(hReportFile,"Fail count,Parametric Failure signatures\n");
            }
            else
            {
                // Display total parts tested
                fprintf(hReportFile,
                        "<br>Total devices tested: <b>%ld</b><br>\n",
                        lTotalPartsTested);
                fprintf(hReportFile,"Total patterns detected: <b>%d</b><br>\n",mParametricFailurePatterns.count());
                OpenParametricFailureSignaturesParetoTable();
            }

            qtFailureSignatureList.clear();
            ldTotalFailCount = 0;
            CMprFailureVector *pVector;
            QMap<QString, int>::const_iterator i = mParametricFailurePatterns.constBegin();
            while (i != mParametricFailurePatterns.constEnd())
            {
                pVector = new CMprFailureVector();
                pVector->strVectorName = i.key();	// List of PTR tests failures in run
                pVector->lFailCount = i.value();	// Fail count
                ldTotalFailCount += pVector->lFailCount;
                qtFailureSignatureList.append(pVector);

                // Move to next pattern detected
                ++i;
            }
            qSort(qtFailureSignatureList.begin(), qtFailureSignatureList.end(), CompareFailureSignature);	// Sort Fail list to generate pareto.

            // Display Top X
            fPercentageOfFails = 0;
            fPercentageReported = 0;
            foreach(ptVectorEntry, qtFailureSignatureList)
            {
                // Keep track of total failing patterns
                fPercentageOfFails = (100.0*ptVectorEntry->lFailCount)/ldTotalFailCount;
                fPercentageReported += fPercentageOfFails;

                if(strOutputFormat=="CSV")
                {
                    fprintf(hReportFile, "%ld,%s\n",
                            ptVectorEntry->lFailCount,
                            ptVectorEntry->strVectorName.toLatin1().data());
                }
                else
                {
                    fprintf(hReportFile,"<tr>\n");
                    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"center\"><b>%ld</b></td>\n",szDataColor,ptVectorEntry->lFailCount);
                    fprintf(hReportFile,"<td width=\"90%%\" bgcolor=%s align=\"left\">%s</td>\n",szDataColor,ptVectorEntry->strVectorName.toLatin1().data());
                    fprintf(hReportFile,"</tr>\n");
                }

                // Check if cut-off limit defined (to only output the first X% of signature failures)
                if(lfParetoFailPatternCutoff >= 0 && lfParetoFailPatternCutoff < fPercentageReported)
                    break;	// Enough failures Reported.
            }

            // Close HTML table
            if(strOutputFormat!="CSV")
                fprintf(hReportFile,"</table>\n");

            // Comment about the cutt-off limit
            if(lfParetoFailPatternCutoff >= 0)
                fprintf(hReportFile,"Shows first %.2g %% of the failure signatures (Defined in Options/'Pareto/Define Failure Signatures cut-off limit')\n",
                lfParetoFailPatternCutoff);
        }
    }
    // Clear liste of pointor
    qDeleteAll(qtFailureSignatureList);
    qtFailureSignatureList.clear();
}

/////////////////////////////////////////////////////////////////////////////
// Prepare Soft/Hard Bin pareto section
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::PrepareSection_BinningParetoReport(QString strBinType)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"Pareto of %s Bins\n",strBinType.toLatin1().constData());
    }
    else
    {
        // HTML Report
        // Title + bookmark
        QString strTitle;
        strTitle.sprintf("Pareto of %s Bins",strBinType.toLatin1().constData());
        QString strBookmark = "Pareto of " + strBinType;
        WriteHtmlSectionTitle(hReportFile,strBookmark,strTitle);

        // Have the ToolBar line written in the HTML file (only when we have a single )
        if(strBinType.startsWith("Software", Qt::CaseInsensitive))
            WriteHtmlToolBar(0,true,"drill_table=soft_bin","Edit Colors","../images/color_palette.png","#_gex_bin_colors.htm");

        else
            WriteHtmlToolBar(0,true,"drill_table=hard_bin","Edit Colors","../images/color_palette.png","#_gex_bin_colors.htm");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Open HTML table for: 'Binning Pareto' (Hardware or Software)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::OpenBinningParetoTable(const char *szBinningType)
{
    QString	strBookmark;

    if(hReportFile == NULL)
        return;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="HTML")
    {
        fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
        strBookmark.sprintf("binning.htm#%s",szBinningType);
    }
    else
    {
        int iCellSpacing;
        if (strOutputFormat=="PDF")
            iCellSpacing = 2;
        else
            iCellSpacing = 1;
        WriteHtmlOpenTable(98,iCellSpacing);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)
        strBookmark.sprintf("#%s",szBinningType);
    }
    fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"12%%\" bgcolor=%s align=\"center\"><b><a name=\"Binning\"></a> <a href=\"%s\">%s Binning</a></b></td>\n",szFieldColor,strBookmark.toLatin1().constData(),szBinningType);
    fprintf(hReportFile,"<td width=\"10%%\"  bgcolor=%s align=\"left\"><b>Bin Name</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"8%%\"  bgcolor=%s align=\"center\"><b>Count</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"8%%\"  bgcolor=%s align=\"center\"><b>Percentage</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"60%%\"  bgcolor=%s align=\"center\"><b>%s Binning Chart</b></td>\n",szDataColor,szBinningType);
    fprintf(hReportFile,"</tr>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Creates the .CSV or HTML file, with Binning Pareto report (Soft or Hard)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::CreateBinningParetoReport(CGexGroupOfFiles *pGroup,CBinning *ptBinList,bool bSoftBin,long lTotalParts)
{
    CBinning *			ptBinCell;	// Pointer to Bin cell
    qtTestListBinning	qtBinningList;
    float				fPercentage;
    const char *		szBinningType;
    long				ltTotalPartsReported=0;
    float				lfTotalPercentageReported=0;

    // Gross die count enabled, use it to overload effective total dies in wafer.
    if(pGroup->cMergedData.grossDieCount() > 0)
    {
        // If we've got multiple groups (comparing testing sites), ensure to use gross die count readjusted on a per/site basis
        lTotalParts = pGroup->cMergedData.grossDieCount();
    }

    if(bSoftBin)
        szBinningType = "Software";
    else
        szBinningType = "Hardware";

    ptBinCell= ptBinList;

    // Open a binning table: If PAT-Man report with multi-sites: do not show individual results per site, only show combined one (all sites merged)
    if(m_pReportOptions->getAdvancedReport() != GEX_ADV_OUTLIER_REMOVAL)
        WriteGroupNameLabel(pGroup);

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if (strOutputFormat=="CSV")
    {
        // ASCII .CSV Report.
        fprintf(hReportFile,"%s Binning#,Bin name,Count,Percentage\n",szBinningType);
    }
    else
    {
        // HTML Report: Open Binning table
        OpenBinningParetoTable(szBinningType);
    }

    // Create the list of Binning counts (do not list empty ones)
    while(ptBinCell != NULL)
    {
        if(ptBinCell->ldTotalCount > 0)
            qtBinningList.append(ptBinCell);

        // Move to next bin in list.
        ptBinCell = ptBinCell->ptNextBin;
    };

    // Sort list
    qSort(qtBinningList.begin(), qtBinningList.end(), CompareBinning);

    // Read list in Descending order (Largest Binning count to smallest)
    int		iBin;
    long	lBinReference=0;	// Bin count of first bin in pareto (higher count)
    long	lBinChartSize;
    char	szPercentage[20];
    QString     lBinName;
    QString		strImage;
    int		iLinesInPage=0;		// When creating flat HTML, must insert page break at smart position (before too many lines are written)
    QStringList qslParetoExcludeBinnings = (m_pReportOptions->GetOption(QString("pareto"), QString("excludebinnings"))).toString().split(QString("|"));
    bool bParetoExcludeFailBins = qslParetoExcludeBinnings.contains(QString("fail"));
    bool bParetoExcludePassBins = qslParetoExcludeBinnings.contains(QString("pass"));

    foreach(ptBinCell, qtBinningList)
    {
        // Check if PASS or FAIL binnings should be ignored
        if(	(!bParetoExcludeFailBins || (ptBinCell->cPassFail != 'F')) &&
            (!bParetoExcludePassBins || (ptBinCell->cPassFail != 'P')))
        {
            // Write page break (ignored if not writing a flat HTML document)
            if(iLinesInPage && (iLinesInPage % iLinesPercentagePerFlatPage == 0)
                    && ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
                )
            {
                // close table
                fprintf(hReportFile,"</table>\n");

                // Insert page break
                WritePageBreak();

                // Reopen table
                OpenBinningParetoTable(szBinningType);
            }

            fPercentage = ((float)100.0*ptBinCell->ldTotalCount)/((float)lTotalParts);
            if((fPercentage < 0.0) || (fPercentage > 100.0) || (lTotalParts <= 0))
            {
                // Happens when corrupted binning data (as seen on LTX Fusion systems!).
                strcpy(szPercentage,GEX_NA);
                fPercentage = 0.0;
            }
            else
                sprintf(szPercentage,"%.1f %%",fPercentage);

            // Update Cumul data
            ltTotalPartsReported += ptBinCell->ldTotalCount;
            lfTotalPercentageReported += fPercentage;

            if(ptBinCell->strBinName.isEmpty())
                lBinName="-";
            else
                lBinName = ptBinCell->strBinName;
            if (strOutputFormat=="CSV")
            {
                fprintf(hReportFile,"%d,%s,%d,%s\n",
                    ptBinCell->iBinValue,
                    lBinName.toLatin1().constData(),
                    ptBinCell->ldTotalCount,
                    szPercentage);
            }
            else
            {
                fprintf(hReportFile,"<tr>\n");
                fprintf(hReportFile,"<td width=\"12%%\" bgcolor=%s align=\"center\"><b>%d</b></td>\n",szFieldColor,ptBinCell->iBinValue);
                fprintf(hReportFile,"<td width=\"10%%\"  bgcolor=%s align=\"left\">&nbsp;%s</td>\n",szDataColor,lBinName.toLatin1().constData());
                fprintf(hReportFile,"<td width=\"8%%\"  bgcolor=%s align=\"center\">%d</td>\n",szDataColor,ptBinCell->ldTotalCount);
                fprintf(hReportFile,"<td width=\"8%%\"  bgcolor=%s align=\"center\">%s</td>\n",szDataColor,szPercentage);
                if(iLinesInPage == 0)
                {
                    // This is the first BIN in the pareto: the highest Bin count.
                    lBinReference = ptBinCell->ldTotalCount;
                }
                iBin = ptBinCell->iBinValue;
                if(iBin > 0xf)
                    iBin = 0xf;	// Only support 16 colors for the Bin Chart !
                lBinChartSize = (400*ptBinCell->ldTotalCount)/lBinReference;
                fPercentage = lBinChartSize/4;
                if(lBinChartSize == 0)
                    lBinChartSize = 1;	// Ensure always a tiny bar is shown !

                // Build Image to use to draw the percentage bar.
                strImage = "../images/" + cDieColor.GetBinNumberImageName(ptBinCell->iBinValue,bSoftBin);                

                bool lDisplaySiteRatio = false;
                if(!ReportOptions.GetOption("binning","siteRatio").toString().compare("enabled"))
                   lDisplaySiteRatio = true;

                if(bSoftBin)
                    WriteHtmlPercentageBar(hReportFile,74,szDataColor,ptBinCell->iBinValue,bSoftBin,strImage,lBinChartSize,fPercentage, "", pGroup->cMergedData.SoftBinSiteCounter()[ptBinCell->iBinValue], lDisplaySiteRatio);
                else
                    WriteHtmlPercentageBar(hReportFile,74,szDataColor,ptBinCell->iBinValue,bSoftBin,strImage,lBinChartSize,fPercentage, "", pGroup->cMergedData.HardBinSiteCounter()[ptBinCell->iBinValue], lDisplaySiteRatio);

                fprintf(hReportFile,"</tr>\n");
            }

            // Keep track of the line count.
            iLinesInPage++;
        }
    }
    // Write Cumul of Bins.
    if (strOutputFormat=="CSV")
    {
        fprintf(hReportFile, "-,Cumul.,%ld,%.1f %%\n", ltTotalPartsReported,lfTotalPercentageReported);
    }
    else
    {
        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"12%%\" bgcolor=%s align=\"center\"><b>Cumul.</b></td>\n",szFieldColor);
        fprintf(hReportFile,"<td width=\"10%%\"  bgcolor=%s align=\"left\"><b>Cumul.</b></td>\n",szDataColor);
        fprintf(hReportFile,
            "<td width=\"8%%\"  bgcolor=%s align=\"center\"><b>%ld</b></td>\n",
            szDataColor, ltTotalPartsReported);
        fprintf(hReportFile,"<td width=\"8%%\"  bgcolor=%s align=\"center\"><b>%.1f%%</b></td>\n",szDataColor,lfTotalPercentageReported);
        fprintf(hReportFile,"<td width=\"74%%\" bgcolor=%s align=\"left\">&nbsp;</td>\n",szDataColor);
        fprintf(hReportFile,"</tr>\n");
    }


    if (strOutputFormat=="CSV")
        fprintf(hReportFile,"\n");
    else
        fprintf(hReportFile,"</table>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Creates ALL pages for the Pareto report
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CreatePages_Pareto(void)
{
    CGexGroupOfFiles *pGroup;
    CGexFileInGroup	 *pFile;
    CTest		*ptTestCell;
    CBinning	*ptBin;
    int			iStatus;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating Pareto page...");

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building Pareto section...");
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();

    // OPTIONS
    QString strParetoSectionOptions = (m_pReportOptions->GetOption(QString("pareto"), QString("section"))).toString();
    QStringList qslParetoSectionOptionList = strParetoSectionOptions.split(QString("|"));

    // Do not create this section if:
    // o section disabled and output format != HTML
    // o HTML-based format and secion is part of the sections to skip
    if (		( (strParetoSectionOptions.isEmpty())
                && (strOutputFormat!="HTML") )
            ||
                ( (strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="ODT")
                  && (m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_PARETO) )
        )
    {
        // Update process bar...
        GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
        return GS::StdLib::Stdf::NoError;
    }

    // If section disabled and output format is HTML, create a HTML file specifying section has been disabled
    if(		(strParetoSectionOptions.isEmpty())
        &&
            (strOutputFormat=="HTML")
        )
    {
        // TODO

        // Update process bar...
        //UpdateProcessBar(false,-1,-1);	// Increment to next step.
        //return GS::StdLib::Stdf::NoError;
    }

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    // Create Pareto reports.
    GSLOG(SYSLOG_SEV_DEBUG, "Preparing Section_Pareto...");
    iStatus = PrepareSection_Pareto(true);
    if(iStatus != GS::StdLib::Stdf::NoError)
        return iStatus;

    // Create CP report section for each group.
    if(qslParetoSectionOptionList.contains(QString("cp")))
    {
        PrepareSection_CpParetoReport();

        QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

        while(itGroupsList.hasNext())
        {
            pGroup		= itGroupsList.next();
            pFile		= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            ptTestCell	= pGroup->cMergedData.ptMergedTestList;

            // - Pareto of tests Cp: sorted by Increasing Cp
            CreateCpParetoReport(pGroup,pFile,ptTestCell);

            // Write page break (ignored if not writing a flat HTML document)
            WritePageBreak();
        };

        // Keep track of total HTML pages written
        lReportPageNumber++;
    }

    // Create CPK report section for each group.
    if(qslParetoSectionOptionList.contains(QString("cpk")))
    {
        PrepareSection_CpkParetoReport();
        QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

        while(itGroupsList.hasNext())
        {
            pGroup		= itGroupsList.next();
            pFile		= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            ptTestCell	= pGroup->cMergedData.ptMergedTestList;

            // - Pareto of tests Cpk: sorted by Increasing Cpk
            CreateCpkParetoReport(pGroup,pFile,ptTestCell);

            // Write page break (ignored if not writing a flat HTML document)
            WritePageBreak();
        };

        // Keep track of total HTML pages written
        lReportPageNumber++;
    }

    // Create FAILURES report section for each group.
    if(qslParetoSectionOptionList.contains(QString("failures")))
    {
        PrepareSection_FailuresParetoReport();
        QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

        while(itGroupsList.hasNext())
        {
            pGroup		= itGroupsList.next();
            if (!pGroup)
            {
               GSLOG(SYSLOG_SEV_WARNING, "found a NULL group");
               continue;
            }
            pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            ptTestCell = pGroup->cMergedData.ptMergedTestList;

            WriteLegend(hReportFile, true);

            // - Pareto of tests failures: sorted from highest failure rate to lowest
            CreateFailuresParetoReport(pGroup,pFile,ptTestCell);

            // Write page break (ignored if not writing a flat HTML document)
            WritePageBreak();
        };

        // Keep track of total HTML pages written
        lReportPageNumber++;
    }

    // Create FAILURES Signatures (MPR) report section for each group.
    if(qslParetoSectionOptionList.contains(QString("failure_signature")))
    {
        PrepareSection_FunctionalFailureSignaturesParetoReport();
        QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

        while(itGroupsList.hasNext())
        {
            pGroup		= itGroupsList.next();
            pFile		= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
            ptTestCell	= pGroup->cMergedData.ptMergedTestList;

            // - Pareto of tests failure Patterns (MPR tests only): sorted from highest failure rate to lowest
            CreateFailureSignaturesParetoReport(pGroup,pFile,ptTestCell);

            // Write page break (ignored if not writing a flat HTML document)
            WritePageBreak();
        };

        // Keep track of total HTML pages written
        lReportPageNumber++;
    }

    // Create SOFT BIN report section for each group.
    if(qslParetoSectionOptionList.contains(QString("soft_bin")))
    {
        PrepareSection_BinningParetoReport("Software");
        QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

        while(itGroupsList.hasNext())
        {
            pGroup		= itGroupsList.next();
            ptTestCell	= pGroup->cMergedData.ptMergedTestList;

            WriteLegend(hReportFile, false);
            // - Software Pareto of Binning: sorted by decreasing order
            ptBin = pGroup->cMergedData.ptMergedSoftBinList;
            CreateBinningParetoReport(pGroup,ptBin,true,pGroup->cMergedData.lTotalSoftBins);

            // Write page break (ignored if not writing a flat HTML document)
            WritePageBreak();
        };

        // Keep track of total HTML pages written
        lReportPageNumber++;
    }

    // Create HARD BIN report section for each group.
    if(qslParetoSectionOptionList.contains(QString("hard_bin")))
    {
        PrepareSection_BinningParetoReport("Hardware");
        QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());

        while(itGroupsList.hasNext())
        {
            pGroup		= itGroupsList.next();
            ptTestCell	= pGroup->cMergedData.ptMergedTestList;

            WriteLegend(hReportFile, false);

            // - Hardware Pareto of Binning: sorted by decreasing order
            ptBin = pGroup->cMergedData.ptMergedHardBinList;
            CreateBinningParetoReport(pGroup,ptBin,false,pGroup->cMergedData.lTotalHardBins);

            // Write page break (ignored if not writing a flat HTML document)
            WritePageBreak();
        };
    }

    iStatus = CloseSection_Pareto();
    return iStatus;
}
