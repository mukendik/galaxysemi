/////////////////////////////////////////////////////////////////////////////
// Creates HTML 'Binning' page.
/////////////////////////////////////////////////////////////////////////////

#include <QLabel>
#include "browser.h"
#include "report_build.h"
#include "report_options.h"
#include "cbinning.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "product_info.h"
#include "engine.h"

// main.cpp
extern CGexReport*		gexReport;				// report_build.cpp: Handle to report class

// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)



/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_SoftBinning(BOOL bValidSection)
{
    char	szString[2048];

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Creates the 'Binning' page & header
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(hReportFile,"\n---- Binning info ----\n\n");
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased()) //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        // Generating HTML report file (and not MyReport...)
        if(	(strOutputFormat=="HTML")
           && m_pReportOptions->strTemplateFile.isEmpty())
        {
            // Open <stdf-filename>/report/binning.htm
            sprintf(szString,"%s/pages/binning.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
            hReportFile = fopen(szString,"wt");
            if(hReportFile == NULL)
                return GS::StdLib::Stdf::ReportFile;
        }

        WriteHeaderHTML(hReportFile,"#000000");	// Default: Text is Black

        // Title + bookmark: "Binning summary"
        if(	(strOutputFormat=="HTML")
           && m_pReportOptions->strTemplateFile.isEmpty())
        {
            WriteHtmlSectionTitle(hReportFile,"all_binnings", m_pReportOptions->GetOption("binning", "section_name").toString());
        }

        if(bValidSection == false)
        {
            fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No Software binning data available !<br>\n",iHthmNormalFontSize);
        }
        else
        {
            // Standard HTML page: create short list of hyperlinks to next sections of paretos.
            if(strOutputFormat=="HTML")
            {
                fprintf(hReportFile,"<align=\"left\">The Binning tables include reports for:<br>\n");
                fprintf(hReportFile,"<blockquote>\n");
                fprintf(hReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#Software\">Software Bins</a><br>\n");
                fprintf(hReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#Hardware\">Hardware Bins</a><br><br>\n");
                fprintf(hReportFile,"</blockquote>\n");
            }

            // Title + bookmark
            WriteHtmlSectionTitle(hReportFile,"Software","Software Binning Summary");

            // Have the ToolBar line written in the HTML file (only when we have a single )
            QString strDrillArgument = "drill_table=soft_bin";
            QString strExportSBL = "#_gex_export_sbl.htm";

            // Allow to export SBL (Static Binning Limits)
            WriteHtmlToolBar(0,true,"drill_table=soft_bin","Edit Colors","../images/color_palette.png","#_gex_bin_colors.htm");
#if 0
            WriteHtmlToolBar(0,true,strDrillArgument,
                        "Edit Colors","../images/color_palette.png","#_gex_bin_colors.htm",
                        "Export YBL/SBL to file","../images/save.png",strExportSBL);
#endif
        }
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_SoftBinning(void)
{
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Prepares the report section to be written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::PrepareSection_HardBinning(BOOL bValidSection)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Appends HARD bin data in the SOFT bin page.
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file...nothing to do at this point!
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased()) //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        // Generating HTML report file.
        if(bValidSection == false)
        {
            fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No Hardware binning data available !<br>\n",iHthmNormalFontSize);
        }
        else
        {
            fprintf(hReportFile,"<br>\n");

            // Title + bookmark
            WriteHtmlSectionTitle(hReportFile,"all_hardbinnings","Hardware Binning Summary");

            // Have the ToolBar line written in the HTML file (only when we have a single )
            WriteHtmlToolBar(0,true,"drill_table=hard_bin","Edit Colors","../images/color_palette.png","#_gex_bin_colors.htm");
        }
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// closes the report section just written (.CSV & .HTML)
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CloseSection_HardBinning(void)
{
    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file: no specific task to perform when section written !
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased()) //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        // close page...if this section was created.
        if((IfHtmlSectionCreated(GEX_HTMLSECTION_BINNING) == true)
            &&	(strOutputFormat=="HTML")
            )
        {
            // Close index page (unless creating one flat HTML file with all sections (eg: when creating a Word document)
            fprintf(hReportFile,C_HTML_FOOTER, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );	// Writes HTML footer Application name/web site string
            fprintf(hReportFile,"</body>\n");
            fprintf(hReportFile,"</html>\n");
            // Close report file...unless we're creating a flat HTML file with other sections (eg: MyReport)
            if(m_pReportOptions->strTemplateFile.isEmpty())
                CloseReportFile();
        }
    }
    return GS::StdLib::Stdf::NoError;
}

/////////////////////////////////////////////////////////////////////////////
// Open HTML table for: 'Binning table' (hardware or software)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::OpenBinningTable(const char *szBinType)
{
    QString	strBookmark;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="HTML")
    {
        fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellspacing=\"1\">\n");
        strBookmark.sprintf("pareto.htm#Pareto of %s",szBinType);
    }
    else
    {
        // HTML code to open table, Width = 98%, cell spacing=1
        int iCellSpacing;
        if (strOutputFormat=="PDF")
            iCellSpacing = 2;
        else
            iCellSpacing = 1;
        WriteHtmlOpenTable(98,iCellSpacing);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)

        strBookmark.sprintf("#Pareto of %s",szBinType);
    }
    fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"12%%\" bgcolor=%s align=\"center\"><b><a name=\"%s\"></a> <a href=\"%s\">%s Binning</a></b></td>\n",szFieldColor,szBinType,strBookmark.toLatin1().constData(),szBinType);
    fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\"><b>Bin Name</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\"><b>Pass/ Fail</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"14%%\" bgcolor=%s align=\"center\"><b>Total count</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"14%%\" bgcolor=%s align=\"center\"><b>Percentage</b></td>\n",szDataColor);
    fprintf(hReportFile,"<td width=\"41%%\" bgcolor=%s align=\"center\"><b>%s Binning Chart</b></td>\n",szDataColor,szBinType);
    fprintf(hReportFile,"</tr>\n");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteBinLines(CGexGroupOfFiles *pGroup,CBinning *ptBinCell,long lTotalBins, const char *szBinType,bool bSoftBin/*=true*/)
{
    // Open a binning table: If PAT-Man report with multi-sites: do not show individual results per site, only show combined one (all sites merged)
    if(m_pReportOptions->getAdvancedReport() != GEX_ADV_OUTLIER_REMOVAL)
        WriteGroupNameLabel(pGroup);	// Write group name + link to GlobalInfo page (HTML)

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        fprintf(hReportFile,"%s Binning,",szBinType);
        fprintf(hReportFile,"Bin Name,");
        fprintf(hReportFile,"Pass/ Fail,");
        fprintf(hReportFile,"Total count,");
        fprintf(hReportFile,"Percentage\n");
    }
    else
    {
        // Not a single binning info available...
        if(ptBinCell == NULL)
        {
            if(strOutputFormat=="CSV")
                fprintf(hReportFile,"None\n");
            else
                fprintf(hReportFile,"<b> None </b><br>");
            return;
        }

        // HTML Report: Open Binning table
        OpenBinningTable(szBinType);
    }

    long	lCumulPassBins=0;
    long	lCumulFailBins=0;
    int		iLinesInPage=0;	// When creating flat HTML, must insert page break at smart position (before too many lines are written)

    // Gross die count enabled, use it to overload effective total dies in wafer.
    if(pGroup->cMergedData.grossDieCount() > 0)
    {
        // If we've got multiple groups (comparing testing sites), ensure to use gross die count readjusted on a per/site basis
        lTotalBins = pGroup->cMergedData.grossDieCount(); //  / getGroupsList().count();
    }

    while(ptBinCell != NULL)
    {
        // Write page break (ignored if not writing a flat HTML document)
        if(iLinesInPage && (iLinesInPage >= iLinesPercentagePerFlatPage)
                && ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
            )
        {
            // close table
            fprintf(hReportFile,"</table>\n");

            // Insert page break
            WritePageBreak();

            // Reopen table
            OpenBinningTable(szBinType);
            iLinesInPage = 1;
        }

        WriteBinLine(ptBinCell,bSoftBin,lTotalBins, pGroup);

        if(toupper(ptBinCell->cPassFail) == 'P')
            lCumulPassBins+= ptBinCell->ldTotalCount;	// Compute cumul of all PASS bins
        if(toupper(ptBinCell->cPassFail) == 'F')
            lCumulFailBins+= ptBinCell->ldTotalCount;	// Compute cumul of all FAIL bins
        ptBinCell = ptBinCell->ptNextBin;
        iLinesInPage ++;
    }

    // Write Cumul of PASS bins
    WriteBinLine(NULL,bSoftBin,lTotalBins, pGroup, true,lCumulPassBins);	// Number of PASS (pass as positive number)

    // Write Cumul of FAIL bins
    WriteBinLine(NULL,bSoftBin,lTotalBins, pGroup,true,-lCumulFailBins);	// Number of FAILS (pass at negative number)

    // Write Cumul of bins (Total bins)
    WriteBinLine(NULL,bSoftBin,lTotalBins, pGroup);

    // Close table + write comment about the 'Options/Binning'
    QString strNote="From the 'Options' tab in the 'Binning' section you can configure how to compute the binning (from summary or samples)";
    if (m_pReportOptions->isReportOutputHtmlBased())    //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        fprintf(hReportFile,"</table>\n");
        if(pGroup->cMergedData.grossDieCount() > 0)
            fprintf(hReportFile,"<b>Note:</b> All percentages computed using wafer GrossDie count of: %d<br>\n", pGroup->cMergedData.grossDieCount());
        fprintf(hReportFile,"<b>Hint: </b>%s<br>\n",strNote.toLatin1().constData());
    }
    else
    {
        if(pGroup->cMergedData.grossDieCount() > 0)
            fprintf(hReportFile,"Note: All percentages computed using wafer GrossDie count of: %d\n", pGroup->cMergedData.grossDieCount());
        fprintf(hReportFile,"%%Hint: %s\n\n",strNote.toLatin1().constData());
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteBinLine(CBinning	*ptBinCell,bool bSoftBin,long lTotalBins, CGexGroupOfFiles* group,bool bPassFailCumul,long lTotalPassOrFail)
{
    char		szBin[20];
    char		szPassFail[2];
    char		szCount[20];
    char		szPercentage[20];
    float		fPercentage;
    int			iBinChartSize;
    int			iBin;
    QString		strImage;
    QString     lBinName;

    // Check if it is an individual Bin entry or the Cumul to display...
    if(ptBinCell != NULL)
    {
        // If bin count = 0, then do not write line!
        if(!ptBinCell->ldTotalCount)
            return;

        if(ptBinCell->strBinName.isEmpty())
            lBinName="-";
        else
            lBinName = ptBinCell->strBinName;

        sprintf(szBin,"%d",ptBinCell->iBinValue);
        iBin = ptBinCell->iBinValue;
        if(iBin > 0xf)
            iBin = 0xf;	// Only support 16 colors for the Bin Chart !
        sprintf(szPassFail,"%c",ptBinCell->cPassFail);
        sprintf(szCount,"%d",ptBinCell->ldTotalCount);
        fPercentage = (100.0*ptBinCell->ldTotalCount)/lTotalBins;
    }
    else
    {
        // Cumul Of Bins: All bins, PASS bins, or FAIL bins.
        if(bPassFailCumul == false)
        {
            strcpy(szBin,"ALL Bins");
            lBinName = szBin;
            strcpy(szPassFail,"-");
            sprintf(szCount,"%ld",lTotalBins);
            fPercentage = 100.0;
            iBin = 0xff;
        }
        else
        {
            // Cumul of PASS (if > 0 number) or FAIL (if < 0 number)
            if(lTotalPassOrFail == 0)
                return;	// Do not display cumul since it is 0!
            if(lTotalPassOrFail > 0)
            {
                strcpy(szBin,"All PASS Bins");
                strcpy(szPassFail,"P");
            }
            else
            {
                strcpy(szBin,"All FAIL Bins");
                strcpy(szPassFail,"F");
                lTotalPassOrFail = std::abs(lTotalPassOrFail);	// remove negative sign!
            }

            lBinName = szBin;
            sprintf(szCount,"%ld",lTotalPassOrFail);
            fPercentage = (100.0*lTotalPassOrFail)/lTotalBins;
            iBin = 0xff;
        }
    }

    // Compute Histogram bar size.
    if((fPercentage < 0.0) || (fPercentage > 100.0) || (lTotalBins <= 0))
    {
        // Happens when corrupted binning data (as seen on LTX Fusion systems!).
        strcpy(szPercentage,GEX_NA);
        fPercentage = 0.0;
    }
    else
        sprintf(szPercentage,"%.1f %%",fPercentage);
    iBinChartSize = (int)(fPercentage*4);
    if(iBinChartSize == 0)
        iBinChartSize=1;

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    if(strOutputFormat=="CSV")
    {
        // Generating .CSV report file.
       fprintf(hReportFile,"%s,%s,%s,%s,%s\n", szBin, lBinName.toLatin1().constData(),
               szPassFail, szCount, szPercentage);
    }
    else
    if(m_pReportOptions->isReportOutputHtmlBased()) //strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
    {
        // Generating .HTML report file.
        fprintf(hReportFile,"<tr>\n");

        if(ptBinCell != NULL)
        {
            fprintf(hReportFile,"<td width=\"12%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,szBin);
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\">&nbsp;%s</td>\n",szDataColor,lBinName.toLatin1().constData());
            fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\">%s</td>\n",szDataColor,szPassFail);
            fprintf(hReportFile,"<td width=\"14%%\" bgcolor=%s align=\"center\">%s</td>\n",szDataColor,szCount);
            fprintf(hReportFile,"<td width=\"14%%\" bgcolor=%s align=\"center\">%s</td>\n",szDataColor,szPercentage);

            // Build Image to use to draw the percentage bar.
            strImage = "../images/" + cDieColor.GetBinNumberImageName(ptBinCell->iBinValue,bSoftBin);

            bool lDisplaySiteRatio = false;
            if(!ReportOptions.GetOption("binning","siteRatio").toString().compare("enabled"))
              lDisplaySiteRatio = true;

            if(bSoftBin)
                WriteHtmlPercentageBar(hReportFile,141,szDataColor,ptBinCell->iBinValue,bSoftBin,strImage,iBinChartSize,fPercentage,"", group->cMergedData.SoftBinSiteCounter()[ptBinCell->iBinValue], lDisplaySiteRatio);
            else
                WriteHtmlPercentageBar(hReportFile,141,szDataColor,ptBinCell->iBinValue,bSoftBin,strImage,iBinChartSize,fPercentage,"", group->cMergedData.HardBinSiteCounter()[ptBinCell->iBinValue], lDisplaySiteRatio);

        }
        else
        {
            // Cumul binning, write it in bold.
            fprintf(hReportFile,"<td width=\"12%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szFieldColor,szBin);
            fprintf(hReportFile,"<td width=\"10%%\" bgcolor=%s align=\"left\">&nbsp;<b>%s</b></td>\n",szDataColor,lBinName.toLatin1().constData());
            fprintf(hReportFile,"<td width=\"7%%\" bgcolor=%s align=\"center\">%s</td>\n",szDataColor,szPassFail);
            fprintf(hReportFile,"<td width=\"14%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,szCount);
            fprintf(hReportFile,"<td width=\"14%%\" bgcolor=%s align=\"center\"><b>%s</b></td>\n",szDataColor,szPercentage);
            fprintf(hReportFile,"<td width=\"41%%\" bgcolor=%s align=\"left\">&nbsp;</td>\n",szDataColor);
        }

        fprintf(hReportFile,"</tr>\n");
    }
}

/////////////////////////////////////////////////////////////////////////////
// Creates ALL pages for the Binning report
/////////////////////////////////////////////////////////////////////////////
int	CGexReport::CreatePages_Binning(void)
{
    // Create Software Binning Summary report
    CBinning	*ptBinCell;	// Pointer to Bin cell
    int			iStatus;

    // Status bar message.
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(" Building Binning section...");

    QString strOutputFormat=m_pReportOptions->GetOption("output", "format").toString();
    // Do not create this section if:
    // o section disabled and output format != HTML
    // o HTML-based format and secion is part of the sections to skip
    {
        QString strOptionStorageDevice;
        strOptionStorageDevice = (m_pReportOptions->GetOption("binning","section")).toString();

        if(	( (strOptionStorageDevice == "disabled")
                && (strOutputFormat!="HTML")
            )
            ||
            ( m_pReportOptions->isReportOutputHtmlBased() //(strOutputFormat=="HTML"||strOutputFormat=="DOC"||strOutputFormat=="PDF"||strOutputFormat=="PPT"||strOutputFormat=="INTERACTIVE")
                && (m_pReportOptions->iHtmlSectionsToSkip & GEX_HTMLSECTION_BINNING)))
        {
            // Update process bar...
            GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.
            return GS::StdLib::Stdf::NoError;
        }

        // If section disabled and output format is HTML, create a HTML file specifying section has been disabled
        if( (strOptionStorageDevice=="disabled") && (strOutputFormat=="HTML") )
        {
            if(m_pReportOptions->strTemplateFile.isEmpty())
            {
                char   szString[2048];
                sprintf(szString,"%s/pages/binning.htm",m_pReportOptions->strReportDirectory.toLatin1().constData());
                hReportFile = fopen(szString,"wt");
                if(hReportFile == NULL)
                {
                    return GS::StdLib::Stdf::ReportFile;
                }
            }
            WriteHeaderHTML(hReportFile,"#000000");    // Default: Text is Black
            WriteHtmlSectionTitle(hReportFile,"all_binnings", m_pReportOptions->GetOption("binning", "section_name")
                                  .toString());
            fprintf(hReportFile,"<p align=\"left\"><font color=\"#000000\" size=\"%d\">"
                    "The Binning report section is disabled from within the options"
                    " (\"Binning->Include section in report ?\")<br>\n",iHthmNormalFontSize);
            CloseReportFile();
            GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);  // Increment to next step.
            return GS::StdLib::Stdf::NoError;
        }

    }

    // Get pointer to first group & first file (we always have them exist)
    QListIterator<CGexGroupOfFiles*>	itGroupsList(getGroupsList());
    CGexGroupOfFiles *					pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
  //FIXME: not used ?
  //CGexFileInGroup* pFile =
  //  (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // In case we only have one group and no SoftBin, then do not even create this section (flat HTML only)!
    if((getGroupsList().count() == 1) && (pGroup->cMergedData.lTotalSoftBins <= 0)
        &&
            ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
        )
        goto hard_bin;

    // Update process bar...
    GS::Gex::Engine::GetInstance().UpdateProgressStatus(false,-1,-1);	// Increment to next step.

    // Opens HTML binning page
    iStatus = PrepareSection_SoftBinning(true);
    if(iStatus != GS::StdLib::Stdf::NoError)
        return iStatus;

    // Create report section for each group.
    while(itGroupsList.hasNext())
    {
        pGroup		= itGroupsList.next();

        // Point to Soft bin list
        ptBinCell	= pGroup->cMergedData.ptMergedSoftBinList;

        // write legend in the case of site ratio displaying
        WriteLegend(hReportFile, false);
        // Write Soft bin section for given group
        WriteBinLines(pGroup,ptBinCell,pGroup->cMergedData.lTotalSoftBins,"Software",true);

        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();

        // If PAT-Man report with multi-sites: do not show individual results per site, only show combined one (all sites merged)
        if((getGroupsList().count() > 1) && (ReportOptions.getAdvancedReport() == GEX_ADV_OUTLIER_REMOVAL))
            break;
    };

    // Close Soft bin section.
    CloseSection_SoftBinning();

hard_bin:
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
  //FIXME: not used ?
  //pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    // In case we only have one group and no SoftBin, then do not even create this section (flat HTML only)!
    if((getGroupsList().count() == 1) && (pGroup->cMergedData.lTotalHardBins <= 0)
        &&
            ( (strOutputFormat=="DOC")||(strOutputFormat=="PDF")||(strOutputFormat=="PPT")||strOutputFormat=="ODT" )
        )
        return GS::StdLib::Stdf::NoError;

    // If PAT-Man report with multi-sites: do not show hard-bins
    if((getGroupsList().count() > 1) && (ReportOptions.getAdvancedReport() == GEX_ADV_OUTLIER_REMOVAL))
        goto end_bins;

    // Rewind to first group...so we now list Hardware binning
    pGroup = getGroupsList().isEmpty()?NULL:getGroupsList().first();
    iStatus = PrepareSection_HardBinning(true);
    if(iStatus != GS::StdLib::Stdf::NoError)
        return iStatus;

    itGroupsList.toFront();

    // Create report section for each group.
    while(itGroupsList.hasNext())
    {
        pGroup		= itGroupsList.next();

        // Point to Hard bin list
        ptBinCell	= pGroup->cMergedData.ptMergedHardBinList;

       // write legend in the case of site ratio displaying
       WriteLegend(hReportFile, false);

        // Write Hard bin section for given group
        WriteBinLines(pGroup,ptBinCell,pGroup->cMergedData.lTotalHardBins,"Hardware",false);

        // Write page break (ignored if not writing a flat HTML document)
        WritePageBreak();
    };

end_bins:
    iStatus = CloseSection_HardBinning();
    return iStatus;
}
