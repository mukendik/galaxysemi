#include <gqtl_sysutils.h>

#include <QPainter>
#include "browser_dialog.h"
#include "gex_report.h"
#include "gex_shared.h"
#include "gex_group_of_files.h"
#include "gex_file_in_group.h"
#include "cmerged_results.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"
#include "csl/csl_engine.h"

#ifdef _WIN32
    #undef CopyFile
#endif

extern CGexReport*     gexReport; // Handle to report class
extern QString			formatHtmlImageFilename(const QString& strImageFileName);
extern GexMainwindow	*pGexMainWindow;

/////////////////////////////////////////////////////////////////////////////
// Write the HTML table of contents (<h1> type) + Quantix icon button + hyperlink (if flat HTML)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteExaminatorTableOfcontents(FILE *hHtmlReportFile)
{
  int		iSections=0;
  bool	bWriteDelimiter;

  // OPTIONS
  QString	strParetoSectionOptions = (m_pReportOptions->GetOption(QString("pareto"), QString("section"))).toString();
  QStringList qslParetoSectionOptionList = strParetoSectionOptions.split(QString("|"));

  // Test statistics link
  if(m_pReportOptions->iStatsType != GEX_STATISTICS_DISABLED)
  {
    fprintf(hHtmlReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#all_stats\">%s</a><br>\n",m_pReportOptions->GetOption("statistics", "section_name").toString().toLatin1().constData());

    // Keep track of sections
    iSections++;
  }

  // Histograms link
  if(m_pReportOptions->iHistogramType != GEX_HISTOGRAM_DISABLED)
  {
    fprintf(hHtmlReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#all_histogram\">%s</a><br>\n",m_pReportOptions->GetOption("histogram", "section_name").toString().toLatin1().constData());

    // Keep track of sections
    iSections++;
  }

  if(!strParetoSectionOptions.isEmpty())
  {
    bWriteDelimiter = false;
    fprintf(hHtmlReportFile,
       "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> Pareto lists: \n");
    if(qslParetoSectionOptionList.contains(QString("cp")))
    {
      if(bWriteDelimiter)
        fprintf(hHtmlReportFile," , ");
      fprintf(hHtmlReportFile,"<a href=\"#Pareto of Tests Cp\">Tests Cp</a>\n");
      bWriteDelimiter = true;
    }
    if(qslParetoSectionOptionList.contains(QString("cpk")))
    {
      if(bWriteDelimiter)
        fprintf(hHtmlReportFile," , ");
      fprintf(hHtmlReportFile,"<a href=\"#Pareto of Tests Cpk\">Tests Cpk</a>\n");
      bWriteDelimiter = true;
    }
    if(qslParetoSectionOptionList.contains(QString("failures")))
    {
      if(bWriteDelimiter)
        fprintf(hHtmlReportFile," , ");
      fprintf(hHtmlReportFile,"<a href=\"#Pareto of Tests failures\">Failures</a>\n");
      bWriteDelimiter = true;
    }
    if(qslParetoSectionOptionList.contains(QString("failure_signature")))
    {
      if(bWriteDelimiter)
        fprintf(hHtmlReportFile," , ");
      fprintf(hHtmlReportFile,"<a href=\"#Pareto of Failure Signatures\">Failure Signatures</a>\n");
      bWriteDelimiter = true;
    }
    if(qslParetoSectionOptionList.contains(QString("soft_bin")))
    {
      if(bWriteDelimiter)
        fprintf(hHtmlReportFile," , ");
      fprintf(hHtmlReportFile,"<a href=\"#Pareto of Software\">Software Bin</a>\n");
      bWriteDelimiter = true;
    }
    if(qslParetoSectionOptionList.contains(QString("hard_bin")))
    {
      if(bWriteDelimiter)
        fprintf(hHtmlReportFile," , ");
      fprintf(hHtmlReportFile,"<a href=\"#Pareto of Hardware\">Hardware Bin</a>\n");
      bWriteDelimiter = true;
    }
    fprintf(hHtmlReportFile,"<br>\n");

    // Keep track of sections
    iSections++;
  }

  // Wafer maps link: "Wafer maps & Strip maps"
  if(m_pReportOptions->iWafermapType != GEX_WAFMAP_DISABLED)
  {
    fprintf(hHtmlReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\">"
            "<a href=\"#all_wafers\">%s</a><br>\n",
            m_pReportOptions->GetOption("wafer", "section_name").toString().toLatin1().constData());

    // Keep track of sections
    iSections++;
  }

  // Binning links
  {
    QString strOptionStorageDevice;
    strOptionStorageDevice = (m_pReportOptions->GetOption("binning","section")).toString();

    if(strOptionStorageDevice=="enabled")
    {
      fprintf(hHtmlReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#Software\">Bins (Software</a> , ");
      fprintf(hHtmlReportFile,"<a href=\"#Hardware\">Hardware</a>)<br>\n");

      // Keep track of sections
      iSections++;
    }
  }

  // Advanced reports
  if(m_pReportOptions->getAdvancedReport() != GEX_ADV_DISABLED)
  {
    fprintf(hHtmlReportFile,
     "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#all_advanced\">Advanced reports (Trends, scatter, R&R, Datalog...)</a><br>\n");
    // Keep track of sections
    iSections++;
  }

  //Add Report Log section
  if(GetReportLogList().count() > 0)
  {
      int lWarningCount = GetReportLogList().filter(GS::Gex::ReportLog::ReportWarning).count();
      int lErrorCount = GetReportLogList().filter(GS::Gex::ReportLog::ReportError).count();
      int lInfoCount =  GetReportLogList().filter(GS::Gex::ReportLog::ReportInformation).count();

      QString lDetails = QString("Message Log : Error(%1) Warning(%2) Information(%3)")
              .arg((lWarningCount>0) ? QString::number(lWarningCount): QString("None"))
              .arg((lErrorCount>0) ? QString::number(lErrorCount): QString("None"))
              .arg((lInfoCount>0) ? QString::number(lInfoCount): QString("None"));

      fprintf(hHtmlReportFile,
       "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#report_log\">%s</a><br>\n", lDetails.toLatin1().constData());
      // Keep track of sections
      iSections++;
  }
  else
  {
      QString lDetails = "Message Log : Empty";
      fprintf(hHtmlReportFile,
       "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#report_log\">%s</a><br>\n", lDetails.toLatin1().constData());
      // Keep track of sections
      iSections++;

  }

  // If at least two sections (including globals) to write, add hyperlink, otherwise no hyperlinks!
  if(iSections)
  {
    // global Info.
    fprintf(hHtmlReportFile,
     "<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#all_globalinfo\">Global information and options</a></p>\n");

    // Close table (used to have all tables of contents Centered).
    fprintf(hHtmlReportFile,"</blockquote>\n<br>");
    fprintf(hHtmlReportFile,"</td>\n</tr>\n</table>\n");

    // Write page break
    WritePageBreak(hHtmlReportFile);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Write the HTML section title (<h1> type) + Quantix icon button + hyperlink (if flat HTML)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteHtmlSectionTitle(FILE *hReportFile,QString strBookmarkName,QString strSectionTitle)
{
  if(hReportFile == NULL)
    return;
  QString of=ReportOptions.GetOption("output", "format").toString();
  if(of=="CSV")
    return;

  if(strBookmarkName.isEmpty() == false)
    fprintf(hReportFile,"<a name=\"%s\"></a>",strBookmarkName.toLatin1().constData());
  fprintf(hReportFile,"<h1 align=\"left\"><font color=\"#006699\">");
  if (of!="HTML")
  {
    // Flat HTML file: Add 'Quantix' button icon before each Section title as hyperlink to Quantix's Web site
    fprintf(hReportFile,"<a href=\"http://www.mentor.com\"><img border=\"0\" src=\"../images/quantix.png\"></a>&nbsp;");
  }

  // Write H1 Title in HTML page...or Examinator Web site if PowerPoint slide (as Slide Title is defined few ines beow with 'SetPowerPointSlideName')
  QString strH1Title = strSectionTitle;
  if(of=="PPT")
  {
    switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
    {
//      case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//        strH1Title = "Yield123";
//        break;

      default:
        strH1Title = "Examinator";
        break;
    }
    strH1Title += " Report - www.mentor.com";
  }

  fprintf(hReportFile,"%s</font></h1>\n",strH1Title.toLatin1().constData());

  // Evaluation Copy notice...
  if(GS::LPPlugin::ProductInfo::getInstance()->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    fprintf(hReportFile,"<align=\"left\"><font color=\"#FF6600\">...%s..</font><br>\n",GEX_EVALCOPY_NOTICE);

  if(of=="HTML")
  {
    fprintf(hReportFile,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\">\n");
    fprintf(hReportFile,"<br>\n");
  }
  // If creating a Powerpoint presentation, save Section title.
  if(of=="PPT")
    SetPowerPointSlideName(strSectionTitle);
}


void	CGexReport::WriteHtmlPercentageBar(FILE *hReportFile,int iCellSizePercentage,QString strDataColor,int iColorMapIndex,bool bSoftBin,QString strImage,int iImageSize,float fPercentage, QString strComment)
{
    CMergedResults::T_SiteCounter lEmptyMap;
    WriteHtmlPercentageBar(hReportFile,iCellSizePercentage,strDataColor,iColorMapIndex,bSoftBin,strImage, iImageSize,fPercentage, strComment, lEmptyMap, false);
}



void CGexReport::buildSiteColorPareto()
{
    mColorsPerSitePareto.append(QColor(0xff,0,0));
    mColorsPerSitePareto.append(QColor(0xff,0xff,0));
    mColorsPerSitePareto.append(QColor(0xff,0,0xff));
    mColorsPerSitePareto.append(QColor(0x84,0x84,0xff));
    mColorsPerSitePareto.append(QColor(0,0x84,0x84));
    mColorsPerSitePareto.append(QColor(0xff,0xc6,0xc6));
    mColorsPerSitePareto.append(QColor(0xc6,0xff,0xc6));
    mColorsPerSitePareto.append(QColor(0x84,0x84,0));
    mColorsPerSitePareto.append(QColor(0,0,0x84));
    mColorsPerSitePareto.append(QColor(0,0xa6,0));
    mColorsPerSitePareto.append(QColor(0,0x86,0));
    mColorsPerSitePareto.append(QColor(0,0x66,0));
    mColorsPerSitePareto.append(QColor(0,0x46,0));
}

QColor CGexReport::getSiteColorPareto(int site)
{
    if(mColorsPerSitePareto.empty())
        buildSiteColorPareto();

    //-- if more sites than colors defined, loop over the existing color
    if(site >= mColorsPerSitePareto.size())
    {
        return mColorsPerSitePareto[site%mColorsPerSitePareto.size()];
    }

    return mColorsPerSitePareto[site];
}

QString CGexReport::CreatePixmap(int total, const CMergedResults::T_SiteCounter &siteProportion)
{
    CMergedResults::T_SiteCounter::const_iterator lIterB(siteProportion.begin()), lIterEnd(siteProportion.end());

    int lCurPos         = 0;
    QPixmap lPixmap( 100, 10 );
    QPainter lPainter(&lPixmap);

    QColor lColor;
    for(lIterB = siteProportion.begin(); lIterB != lIterEnd; ++lIterB)
    {
        int lCellSize = ((((float)lIterB.value()*100.)/(float)total) + 0.5);

        // use custom color or default
        if(ReportOptions.bUseCustomBinColors)
            lColor =  GetColorSite(lIterB.key());
        else
        {
            lColor = getSiteColorPareto(lIterB.key());
        }

        lPainter.fillRect(lCurPos, 0, lCellSize, 10, lColor);
        lCurPos += lCellSize;
    }

    // -- due to round, may left some black pixel column.
    if(lCurPos < 100)
        lPainter.fillRect(lCurPos, 0, (100 - lCurPos) , 10, lColor);

    QString lRBGFile = QString("rgb_%1%2.png").arg(GS::Gex::Engine::GetInstance().GetClientDateTime().toTime_t()).arg(mIndicePixmap);
    QString lFileToSave = ReportOptions.strReportDirectory + "/images/" + lRBGFile;

    ++mIndicePixmap;
    if(lPixmap.save(lFileToSave) )
    {
        QString lHTMLFile = "../images/" + lRBGFile;
        return  formatHtmlImageFilename(lHTMLFile);
    }

    return "";
}


void CGexReport::WriteLegend(FILE *hReportFile, bool pareto)
{
    // -- the legend is needed only when the option site ratio has been set for binning or for pareto

    if( (pareto == false && !ReportOptions.GetOption("binning","siteRatio").toString().compare("enabled")) ||
         (pareto == true && !ReportOptions.GetOption("pareto","siteRatio").toString().compare("enabled"))
         )
    {
        QString of=ReportOptions.GetOption("output", "format").toString();
        if (of=="HTML")
        {
            fprintf(hReportFile,"<table border=\"0\" width=\"98%%\" cellpadding=\"0\">\n");
        }
        else
        {
            WriteHtmlOpenTable(98,0);
        }
        fprintf(hReportFile,"<tr>\n");

        CGexGroupOfFiles *pGroup = pGroupsList.isEmpty()?NULL:pGroupsList.first();

        QList<int> lListSites = pGroup->cMergedData.GetSitesList().toList();
        qSort(lListSites);
        QList<int>::iterator lIterB(lListSites.begin()), lIterE(lListSites.end());

        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Site</b></td>\n",szFieldColor);

        for(; lIterB != lIterE; ++lIterB)
        {
            if(*lIterB == 255)
                break;
           fprintf(hReportFile,"<td height=\"21\" bgcolor=%s align=\"center\">%d</td>\n",szDataColor, *lIterB);
        }
        fprintf(hReportFile,"</tr>\n");

        fprintf(hReportFile,"<tr>\n");
        fprintf(hReportFile,"<td width=\"21%%\" bgcolor=%s align=\"left\"><b>Color</b></td>\n",szFieldColor);
        lIterE = lListSites.end();

        for(lIterB = lListSites.begin(); lIterB != lIterE; ++lIterB)
        {
            if(*lIterB == 255)
                break;

            QColor lColor;
            // use custom color or default
            if(ReportOptions.bUseCustomBinColors)
                lColor =  GetColorSite(*lIterB);
            else
                lColor = getSiteColorPareto(*lIterB);

            fprintf(hReportFile,"<td height=\"21\" bgcolor=\"%s\" bordercolor=\"#000000\">&nbsp;</td>\n", lColor.name().toLatin1().constData()) ;
        }
        fprintf(hReportFile,"</tr>\n");
        fprintf(hReportFile,"</table>\n<br>\n");
    }
}


QColor CGexReport::GetColorSite(int indexColor/*, CGexReport::ColorType colorType*/)
{
    QColor	lRgbColor(Qt::black);
    // Check if use default colors or custom colors
    if(ReportOptions.bUseCustomBinColors)
    {
        QList <CBinColor>::iterator itBegin = ReportOptions.siteColorList.begin();
        QList <CBinColor>::iterator itEnd	= ReportOptions.siteColorList.end();
        for( ; itBegin != itEnd; ++itBegin)
        {
            if((*itBegin).cBinRange->Contains(indexColor))
            {
                lRgbColor = (*itBegin).cBinColor;
                break;
            }
        }
    }

    return lRgbColor;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteHtmlPercentageBar(FILE *hReportFile,int iCellSizePercentage,QString strDataColor,int iColorMapIndex,bool bSoftBin,QString strImage,int iImageSize,float fPercentage , QString strComment, const CMergedResults::T_SiteCounter &siteProportion, bool displaySiteRation)
{
  QString of=ReportOptions.GetOption("output", "format").toString();
  if(of=="HTML" || of =="PDF")
  {
      // QString toDisplay = formatHtmlImageFilename(strImage);
      // Standard HTML page: use & resize an image to create the percentage bar
      if(displaySiteRation && !siteProportion.isEmpty())
      {
          int nbTotal = 0.;
          CMergedResults::T_SiteCounter::const_iterator lIterB(siteProportion.begin()), lIterEnd(siteProportion.end());
          for(; lIterB != lIterEnd; ++lIterB )
          {
              nbTotal += *lIterB;
          }

          if(nbTotal == 0)
              return;

          QString lPixmpaToDisplay = CreatePixmap(nbTotal, siteProportion);
          if(!lPixmpaToDisplay.isEmpty())
              fprintf(hReportFile,"<td width=\"%d%%\"><img border=\"1\" src=\"%s\" width=\"%d\" height=\"10\"></td>\n", iCellSizePercentage, lPixmpaToDisplay.toLatin1().constData(), iImageSize);
      }
      else
      {
        fprintf(hReportFile,"<td width=\"%d%%\" bgcolor=%s align=\"left\"><img border=\"1\" src=\"%s\" width=\"%d\" height=\"10\"></td>\n",
            iCellSizePercentage, strDataColor.toLatin1().constData(), formatHtmlImageFilename(strImage).toLatin1().constData(), iImageSize);

      }
  }
  else
  {
    // Flat HTML file created to build a WORD or PDF file: then do not use image resized (not supported well by Word), use table cells instead
    int iCell1,iCell2;
    QString strHtmlColor;
    // Compute the size of Cell1 & Cell2 knowin that Cell1+Cell2 = 100%
    if(fPercentage <= 0)
      iCell1= 1;
    else
    if(fPercentage > 100)
      iCell1 = 100;
    else
      iCell1 = (int)fPercentage;
    iCell2 = 100 - iCell1;

    // Colors associated with binxx.png files in the report/images folder
    switch(iColorMapIndex)
    {
      case 0xff:
        strHtmlColor = "#FFFFFF";
        break;
      case 0x0:
        strHtmlColor = "#0000FF";
        break;
      case 0x1:
        strHtmlColor = "#00C600";
        break;
      case 2:
      default:
        strHtmlColor = "#FF0000";
        break;
      case 0x3:
        strHtmlColor = "#C60000";
        break;
      case 0x4:
        strHtmlColor = "#FFFF00";
        break;
      case 0x5:
        strHtmlColor = "#FF00FF";
        break;
      case 0x6:
        strHtmlColor = "#840084";
        break;
      case 0x7:
        strHtmlColor = "#0000FF";
        break;
      case 0x8:
        strHtmlColor = "#000084";
        break;
      case 0x9:
        strHtmlColor = "#840084";
        break;
      case 0xa:
        strHtmlColor = "#FFC6C6";
        break;
      case 0xb:
        strHtmlColor = "#C6FFC6";
        break;
      case 0xc:
        strHtmlColor = "#008484";
        break;
      case 0xd:
        strHtmlColor = "#848400";
        break;
      case 0xe:
        strHtmlColor = "#840000";
        break;
      case 0xf:
        strHtmlColor = "#8484FF";
        break;
    }

    // If the caller specified a RGB image, then we must use the custom colors, and not the predifined above.
    if(strImage.count("/rgb_", Qt::CaseInsensitive) > 0)
    {
      strHtmlColor = cDieColor.GetBinColorHtmlCode(iColorMapIndex,bSoftBin);
    }

    fprintf(hReportFile,"<td width=\"%d%%\" bgcolor=%s align=\"left\">\n",iCellSizePercentage,strDataColor.toLatin1().constData());
    fprintf(hReportFile,"<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n<tr>\n");
    fprintf(hReportFile,"<td width=\"%d%%\" bgcolor=\"%s\" bordercolor=\"#000000\" style=\"border: 1px solid #000000\"> &nbsp;</td>\n",iCell1,strHtmlColor.toLatin1().constData());
    fprintf(hReportFile,"<td width=\"%d%%\">&nbsp;",iCell2);
    if(fPercentage == 200)
      fprintf(hReportFile,"<img border=\"1\" src=\"../images/bari.png\" width=\"10\" height=\"10\">");
    fprintf(hReportFile,"%s</td>\n",strComment.toLatin1().constData());
    fprintf(hReportFile,"</tr>\n</table>\n</td>");
  }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CGexReport::WriteHtmlToolBar(int iSizeX,bool bAllowInteractive,QString strArgument,QString strText2,QString strImage2,QString strLink2,QString strText3, QString strImage3, QString strLink3)
{
#if 0
  // For debug
  return;
#endif

  QString of=ReportOptions.GetOption("output", "format").toString();
  // If output is WORD or PDF, then do not include interactive hyperlinks in HTML page.
  if(of!="HTML")	//(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_HTML)
    return;

  // Examinator ADVANCED edition: Allows Interactive Drill/Zoom, etc....
  if(iSizeX <=0)
    iSizeX = 850;
  else
  if(iSizeX <= 300)
    iSizeX = 300;

  // Write Zoom-In/Zoom-out toolbar
  fprintf(hReportFile,"<table border=\"0\" width=\"%d\">\n",iSizeX); // Report Image width that follows the toolbar
  fprintf(hReportFile,"<tr>\n<td width=\"95%%\">\n");

  // If only one hyperlink, align to the right, otherwise (if multiple hyperlinks & texts, align left)
  if(strText2.isEmpty() && strText3.isEmpty())
    fprintf(hReportFile,"<p align=\"right\">\n");
  else
    fprintf(hReportFile,"<p align=\"left\">\n");

  // Display 'Interactive 2D/3D' hyperlink
  if(bAllowInteractive)
  {
      fprintf(hReportFile,"<a href=\"#_gex_drill--%s\"><img src=\"../images/zoom_in.png\" border=\"0\"><b>Data Explorer-2D/3D</b></a> (Interactive Zoom, Drill,...)",strArgument.toLatin1().constData());
  }

  // If argument#2
  if(strText2.isEmpty() == false)
    fprintf(hReportFile,"&nbsp;&nbsp;<a href=\"%s\"><img src=\"%s\" border=\"0\"><b>%s</b></a>",strLink2.toLatin1().constData(), formatHtmlImageFilename(strImage2).toLatin1().constData(), strText2.toLatin1().constData());

  // If argument#3
  if(strText3.isEmpty() == false)
    fprintf(hReportFile,"&nbsp;&nbsp;<a href=\"%s\"><img src=\"%s\" border=\"0\"><b>%s</b></a>",strLink3.toLatin1().constData(), formatHtmlImageFilename(strImage3).toLatin1().constData(), strText3.toLatin1().constData());

  // Close toolbar table.
  fprintf(hReportFile,"</td>\n</tr>\n</table>\n");
}

void CGexReport::WriteHtmlOpenTable(int iWidth,int iCellSpacing)
{
  if(hReportFile)
    fprintf(hReportFile,
      "<table border=\"0\" cellspacing=\"%d\" width=\"%d%%\" style=\"font-size:%dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",
      iCellSpacing,iWidth,iHthmSmallFontSizePixels);
}


void CGexReport::WritePageBreak(FILE *hFile,bool bOpenHtml)
{
  QString of=ReportOptions.GetOption("output", "format").toString();
  // Check if creating a flat HTML file (then only one header), or multiple pages?
  if( (of!="DOC")&&(of!="PDF")&&(of!="PPT")&&(of!="ODP")&&(of!="ODT") )
    return;	// We are creating a flat HTML (for Word or PDF), and this is not the First call to this header function!

  FILE  *hFileHandle=0;
  if(hFile != NULL)
    hFileHandle = hFile;
  else
    hFileHandle = hReportFile;

  if(hFileHandle == NULL)
    return;	// Can't write to file!

  // Page break management
  if(of=="PPT"||of=="ODP")	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PPT)
  {
      // Creating PowerPoint slides: create one image for each page and save it as .png
      QString lRes=WritePowerPointSlide(hFileHandle,"",bOpenHtml);
      if (lRes.startsWith("err"))
          GSLOG(SYSLOG_SEV_ERROR, lRes.toLatin1().data());
  }
  else
  {
    // Flat HTML file (for Word or PDF), then insert a HTML page break code.
    //		fprintf(hFileHandle,"\n<p>\n<br>\n<br clear=\"all\" style=\"page-break-before:always\" />\n</p>");
    fprintf(hFileHandle,"\n<p> <br clear=\"all\" style=\"page-break-before:always\" /></p>\n");
  }

  lReportPageNumber++; // Update report page ID
}

void CGexReport::WriteHeaderHTML(FILE *hHtmlReportFile,const char *szColor,const char *szBackground/*="#FFFFFF"*/,QString strHeadLines/*=""*/,bool bReportFile/*=true*/,bool bWriteTOC/*=true*/)
{
  if (!hHtmlReportFile)
  {
      GSLOG(SYSLOG_SEV_WARNING, "hHtmlReportFile NULL");
      //return;
  }
  QString of=ReportOptions.GetOption("output", "format").toString();
  // Check if creating a flat HTML file (then only one header), or multiple pages?
  if(	(of!="HTML") //(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_HTML)
    && lReportPageNumber)
    return;	// We are creating a flat HTML (for Word or PDF), and this is not the First call to this header function!

  // See what kind of HTML file to create (flat, for Word, PDF, Powerpoint,...)
  QString		strTitle;
  bool		bHtmlText=false;
  /*
  switch(pReportOptions->iOutputFormat)
  {
    case	GEX_OPTION_OUTPUT_HTML:
    case	GEX_OPTION_OUTPUT_WORD:
    case	GEX_OPTION_OUTPUT_PDF:
  */
  if (of=="HTML"||of=="DOC"||of=="PDF"||of=="ODT")
      bHtmlText=true;	// Fall thru next case!

  if (of=="CSV")
    //case	GEX_OPTION_OUTPUT_CSV:
  {
      if(bHtmlText)
        fprintf(hHtmlReportFile,"<html>\n");
      fprintf(hHtmlReportFile,"<!-- ***************************************************************************-->\n");
      fprintf(hHtmlReportFile,"<!-- %s-->\n",GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
      fprintf(hHtmlReportFile,"<!-- %s.   www.mentor.com-->\n",C_STDF_COPYRIGHT);
      fprintf(hHtmlReportFile,"<!-- Quantix Examinator - the STDF detective-->\n");
      fprintf(hHtmlReportFile,"<!-- All rights reserved. Users of the program must be Registered Users (see-->\n");
      fprintf(hHtmlReportFile,"<!-- the Quantix examinator license terms).-->\n");
      fprintf(hHtmlReportFile,"<!-- The Quantix Examinator program (including its HTML pages and .png files)-->\n");
      fprintf(hHtmlReportFile,"<!-- is protected by copyright law and international treaties. You are no-->\n");
      fprintf(hHtmlReportFile,"<!-- allowed to alter any HTML page, .png file, or any other part of the-->\n");
      fprintf(hHtmlReportFile,"<!-- software. Unauthorized reproduction or distribution of this program,-->\n");
      fprintf(hHtmlReportFile,"<!-- or any portion of it, may result in severe civil and criminal -->\n");
      fprintf(hHtmlReportFile,"<!-- penalties, and will be prosecuted to the maximum extent possible.-->\n");
      fprintf(hHtmlReportFile,"<!-- ***************************************************************************-->\n");
      if(bHtmlText)
      {
        fprintf(hHtmlReportFile,"<head>\n");
        fprintf(hHtmlReportFile,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n");
      }

      // PDF header string (except on front page)
      if(of=="PDF")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_PDF)
      {
        // Document TITLE is the 'Header' of each page when document is converted to PDF!
        QString strHeader,strFooter;
        BuildHeaderFooterText(strTitle,strTitle);
      }
      else
        strTitle.sprintf("Report created with: %s - www.mentor.com", GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
      if(bHtmlText)
        fprintf(hHtmlReportFile,"<title>%s</title>\n", strTitle.toLatin1().constData());

      // Additional Header lines (if need be)
      if(strHeadLines.isEmpty() == false)
        fprintf(hHtmlReportFile,"%s",strHeadLines.toLatin1().constData());
      if(bHtmlText)
        fprintf(hHtmlReportFile,"</head>\n");
      // Sets default background color & text color given in argument.
      if(bHtmlText)
        fprintf(hHtmlReportFile,"<body bgcolor=\"%s\" text=\"%s\">\n",szBackground,szColor);	// Default color in HTML page!
      //break;
  }
  else if (of=="PPT")
    //case	GEX_OPTION_OUTPUT_PPT:	// Powerpoint slides output
      WritePowerPointBegin();		// Preparation work to begin PowerPoint slides creation.
      //break;
  else if(of=="INTERACTIVE")	//case	GEX_OPTION_OUTPUT_INTERACTIVEONLY:	// Interactive mode, the only HTML page created is for the test statistics when on "Interactive-Chart" page.
      return;
  //}

  // Create Table of content if this is a normal FLAT Html report file (not a status file from Interactive mode)
  if((lReportPageNumber==0) && bReportFile)
    WriteTableOfContents_UserFlow(hHtmlReportFile,bWriteTOC);
}

/////////////////////////////////////////////////////////////////////////////
// Writes at the end of the HTML page the navigation 'Prev' 'Up' 'Next'
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteNavigationButtons(const char *szPageName, bool bTop /* = false */)
{
  char	szString[30];

  QString of=ReportOptions.GetOption("output", "format").toString();
  // If creating a WORD or PDF file (out from a flat HTML), do not include HTML page navigation buttons!
  if( (of=="DOC")||(of=="PDF")||(of=="PPT")||(of=="ODT")||(of=="ODP") ) 	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
    return;

  fprintf(hReportFile,"<table border=\"0\" width=\"98%%\">\n");
  fprintf(hReportFile,"<tr>\n");
  if(iCurrentHtmlPage == 1)
    strcpy(szString,szPageName);
  else
    sprintf(szString,"%s%d",szPageName,iCurrentHtmlPage-1);

  fprintf(hReportFile,"<td width=\"33%%\"><a href=\"%s.htm\"><img src=\"../images/prev.png\" alt=\"Previous report page\" border=\"0\" width=\"58\" height=\"31\"></a></td>\n",
    szString);

  if (bTop)
    // Middle table cell (empty in header)
    fprintf(hReportFile,"<td width=\"33%%\">&nbsp;</td>\n");
  else
    fprintf(hReportFile,"<td width=\"33%%\"><p align=\"center\"><a href=\"%s%d.htm\"><img src=\"../images/top.png\" alt=\"Top of page\" border=\"0\" width=\"58\" height=\"31\"></a></td>\n",
      szPageName,iCurrentHtmlPage);
  // If pointing on last histogram page, 'next' button rollsback to the Home page
  if(iCurrentHtmlPage == mTotalHtmlPages)
    strcpy(szString,szPageName);
  else
    sprintf(szString,"%s%d",szPageName,iCurrentHtmlPage+1);
  fprintf(hReportFile,"<td width=\"34%%\"><p align=\"right\"><a href=\"%s.htm\"><img src=\"../images/next.png\" alt=\"Next report page\" border=\"0\" width=\"51\" height=\"30\"></a></td>\n",
    szString);
  fprintf(hReportFile,"</tr>\n</table>\n");
}

/////////////////////////////////////////////////////////////////////////////
// Writes the Header page + "Table Of Contents" in the HTML page (for sections defined in Examinator 'Settings' page)
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteTableOfContents_UserFlow(FILE *hHtmlReportFile,bool bWriteTOC/*=true*/)
{
  QString of=ReportOptions.GetOption("output", "format").toString();
  // Table of content is only for flat HTML files (WORD, PDF, etc...)
  if(	(of=="HTML") //pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML
    || (of=="CSV")	//pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV
         )
    return;

  // Table of contents is only written if the report is resulting from a scriot execution. If currently in Interactive mode (not scrpting running) do not write any TOC!
  if(GS::Gex::CSLEngine::GetInstance().IsRunning() == false)
    return;

  QString strTitle;
  QString	strAppName;
  bool	bDisplayTitleHeader=true;	// 'false' if the title + time will be later decied (eg: by the report function itself)
  switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
  {
//    case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//      strAppName = "Yield123";
//      bDisplayTitleHeader = false;	// No title, let the Yield123 report function decide itself (eg: 'yield123 Repeatability' etc...)
//      strTitle = "Quantix Yield123";
//      break;

    case GS::LPPlugin::LicenseProvider::ePATMan:		// PAT-Man
    case GS::LPPlugin::LicenseProvider::ePATManEnterprise:
      strTitle = "Quantix PAT-Man reports";
      strAppName = "PAT-Man";
      break;

    case GS::LPPlugin::LicenseProvider::eYieldMan:      // Yield-Man
    case GS::LPPlugin::LicenseProvider::eYieldManEnterprise:
      strTitle = "Quantix reports";
      strAppName = "Yield-Man";
      break;

    default:
      strTitle = "Quantix Examinator reports";
      strAppName = "Examinator";
      // Check if PAT option is set...
      if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
        strTitle = "Examinator-PAT reports";
      break;
  }

  // Set Slide Title
  SetPowerPointSlideName(strTitle);

  // Open table to fit all the Table of Content text
  WriteHtmlOpenTable(98,0);	// HTML code to open table, Width 98%, cell spacing=0
  fprintf(hHtmlReportFile,"<tr>\n");
  fprintf(hHtmlReportFile,"<td align=");

  // If PPT file, center all text in the table, otherwise, left justified
  if(of=="PPT")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_PPT)
    fprintf(hHtmlReportFile,"\"center\">\n");
  else
    fprintf(hHtmlReportFile,"\"left\">\n");

  // Write slide title (900 pixel wide)
  fprintf(hHtmlReportFile,"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" style=\"border-collapse: collapse\" bordercolor=\"#111111\" width=\"900\" bgcolor=\"#F8F8F8\">\n");
  fprintf(hHtmlReportFile,"<tr>\n");
  fprintf(hHtmlReportFile," <td width=\"100%%\"><img border=\"0\" src=\"../images/quantix.png\"></td>\n");
  fprintf(hHtmlReportFile,"</tr>\n");
  fprintf(hHtmlReportFile,"<tr>\n");
  fprintf(hHtmlReportFile,"     <td><p align=\"center\"><b><font face=\"Arial\" size=\"6\" color=\"#0070C0\">Welcome to %s!</font></b></p></td>\n",strTitle.toLatin1().constData());
  fprintf(hHtmlReportFile,"</tr>\n");
  fprintf(hHtmlReportFile,"</table>\n");


  if(bDisplayTitleHeader)
  {
    // Sets default background color = white, text color given in argument.
    time_t iTime = time(NULL);
    fprintf(hHtmlReportFile,"<align=\"left\"><font color=\"#000000\" size=\"%d\"><b>Date: %s</b><br></font>\n",iHthmNormalFontSize,ctime(&iTime));

      // If only one file in report, then display few information such as: Product type, Lot ID
    if(ReportOptions.iFiles == 1)
    {
      CGexGroupOfFiles *pGroup=NULL;
      CGexFileInGroup *pFile=NULL;
      pGroup= getGroupsList().isEmpty()?NULL:getGroupsList().first();
      if(pGroup != NULL)
        pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

      if(pFile != NULL)
      {
        fprintf(hHtmlReportFile,"<align=\"left\"><font color=\"#000000\" size=\"%d\"><b>Product :</b> %s<br></font>\n",iHthmNormalFontSize,pFile->getMirDatas().szPartType);
        fprintf(hHtmlReportFile,"<align=\"left\"><font color=\"#000000\" size=\"%d\"><b>LotID :</b> %s<br></font>\n",iHthmNormalFontSize,pFile->getMirDatas().szLot);
      }

    }

  }
  else
    fprintf(hHtmlReportFile,"<br><br>\n");

  // Write Examinator release (except if PDF file which already writes each page header with this string)
  if(of!="PDF")	//(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_PDF)
    fprintf(hHtmlReportFile,
            "<align=\"left\"><font color=\"#000000\" size=\"%d\"><b>Report created with: </b>%s - www.mentor.com<br><br><br></font>\n",
            iHthmNormalFontSize, GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );

  //******** User Template Text + Logo (if defined....)
  QString fpi=m_pReportOptions->GetOption("output", "front_page_image").toString();
  QString fpt=m_pReportOptions->GetOption("output","front_page_text").toString();

  bool bUserLogo = QFile::exists(fpi); //bool bUserLogo = QFile::exists(pReportOptions->strFrontPageImage);
  bool bUserText = !(fpt.isEmpty()); //bool bUserText = !pReportOptions->strFrontPageText.isEmpty();

  if (bUserLogo || bUserText)
  {
    WriteHtmlOpenTable(98,8);	// HTML code to open table, Width 98%, cell spacing=8
    fprintf(hHtmlReportFile,"<tr >\n");

    if (bUserLogo)
    {
      QImage cImage(fpi); //QImage cImage(pReportOptions->strFrontPageImage);
      QFileInfo cFileInfo(fpi); //QFileInfo cFileInfo(pReportOptions->strFrontPageImage);
      QString strToFile = ReportOptions.strReportDirectory + "/images/" + cFileInfo.fileName();
      CGexSystemUtils::CopyFile(fpi, strToFile); //toolCopyGivenFile(pReportOptions->strFrontPageImage, strToFile);
      fprintf(hHtmlReportFile,"<td align=\"left\">\n");
      fprintf(hHtmlReportFile,"<img border=\"0\" src=\"../images/%s\" width=\"%d\" height=\"%d\">\n", formatHtmlImageFilename(cFileInfo.fileName()).toLatin1().constData(), cImage.width(), cImage.height());
      fprintf(hHtmlReportFile,"</td>\n");
      fprintf(hHtmlReportFile,"</tr><tr>");
    }

    if(bUserText)
    {
      // Text exists...write it!
      fprintf(hHtmlReportFile,"<td align=\"left\" valign=\"top\">\n<font color=\"#000000\" size=\"%d\">",iHthmSmallFontSize);
      fprintf(hHtmlReportFile,"%s", m_pReportOptions->GetOption("output", "front_page_text").toString().toLatin1().constData()); //fprintf(hHtmlReportFile,"%s", pReportOptions->strFrontPageText.toLatin1().constData());
      fprintf(hHtmlReportFile,"</font></td></tr>\n<tr>");
    }

    //******** Close Template table
    fprintf(hHtmlReportFile,"</tr>\n</table>\n<br>");
  }
//	else
//	{
//		fprintf(hHtmlReportFile,"Customize this text...and insert your company logo!<br>");
//		fprintf(hHtmlReportFile,"Click the %s '<b>Options</b>' tab and select '<b>Report output / Home Page Contents</b>'<br>\n",strAppName.toLatin1().constData());
//	}


  //******** Close table
  fprintf(hHtmlReportFile,"</td>\n</tr>\n</table>\n");

  WriteHtmlOpenTable(98,0);	// HTML code to open table, Width 98%, cell spacing=0
  fprintf(hHtmlReportFile,"<tr>\n");
  fprintf(hHtmlReportFile,"<td align=left>");

  if(bWriteTOC)
  {
    fprintf(hHtmlReportFile,"<font color=\"#000000\" size=\"%d\"><br><b>Table of contents</b><br></font>\n",iHthmNormalFontSize);

    fprintf(hHtmlReportFile,"<blockquote>\n");
    fprintf(hHtmlReportFile,"<p align=\"left\">\n");

    WriteExaminatorTableOfcontents(hHtmlReportFile);	// No plugin loaded!...write standard Examinator report 'table of contents'
  }
}

void	CGexReport::WriteInfoLine(FILE *hFile,const char *szLabel, const char *szData)
{
  FILE *tmpFile = hReportFile;
  hReportFile = hFile;
  WriteInfoLine(szLabel,szData);
  hReportFile = tmpFile;
}

void	CGexReport::WriteInfoLine(FILE *hFile,const char *szLabel,int iData)
{
  char	szString[100];

  sprintf(szString,"%99d",iData);
  WriteInfoLine(hFile,szLabel,szString);
}

void	CGexReport::WriteInfoLine(const char *szLabel, const char *szData)
{
  // Skip spaces if any.
  if(szData != NULL)
  {
    while((*szData == ' ') || (*szData == '\t'))
      szData++;
    //if(*szData == 0)
    //	return;	// Empty string !
  }

  QString of=ReportOptions.GetOption("output", "format").toString();
  if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
  {
    if(szLabel == NULL)
    {
      // Display a separator line
      fprintf(hReportFile,"------------------,------------------------------------");
    }
    else
    {
      // Display given parameters
      if(szData == NULL)
        fprintf(hReportFile,"%s,------------------------------------",szLabel);
      else if(*szData == 0)
        fprintf(hReportFile,"%s,n/a",szLabel);
      else
        fprintf(hReportFile,"%s,%s",szLabel,szData);
    }
    // Add CR-LF if not already in the argument.
    if((szData == NULL) || (strchr(szData,'\n') == NULL))
      fprintf(hReportFile,"\n");
  }
  else
      if (m_pReportOptions->isReportOutputHtmlBased())
          //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
          //(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
  {
    fprintf(hReportFile,"<tr>\n");
    if(szLabel == NULL)
    {
      // Display a separator line
      fprintf(hReportFile,"<td width=\"160\" height=\"21\" bgcolor=%s><b><HR></b></td>\n",szFieldColor);
      fprintf(hReportFile,"<td width=\"590\" height=\"21\" bgcolor=%s><HR></td>\n",szDataColor);
    }
    else
    {
      // Display given parameters
      fprintf(hReportFile,"<td width=\"160\" height=\"21\" bgcolor=%s><b>%s</b></td>\n",szFieldColor,szLabel);
      if(szData == NULL)
        fprintf(hReportFile,"<td width=\"590\" height=\"21\" bgcolor=%s><HR></td>\n",szDataColor);
      else if(*szData == 0)
        fprintf(hReportFile,"<td width=\"590\" height=\"21\" bgcolor=%s>n/a</td>\n",szDataColor);
      else
        fprintf(hReportFile,"<td width=\"590\" height=\"21\" bgcolor=%s>%s</td>\n",szDataColor,szData);
    }
    fprintf(hReportFile,"</tr>\n");
  }
}

bool CGexReport::BeginTable(QString atts)
{
    if (!m_pReportOptions)
        return false;
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of!="CSV")
        fprintf(hReportFile,"<table %s >\n", atts.toLatin1().data());
    return true;
}

bool CGexReport::EndTable()
{
    if (!m_pReportOptions)
        return false;
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of!="CSV")
        fprintf(hReportFile,"</table>\n");
    return true;
}

bool CGexReport::EndLine()
{
    if (!m_pReportOptions || !hReportFile)
        return false;
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of!="CSV")
        fprintf(hReportFile,"<br>\n");
    else
        fprintf(hReportFile,"\n");
    return true;
}

bool  CGexReport::WriteTableRow(const QList<QString> &l, const QString &atts, const QList<QString> &anchors)
{
    if (!m_pReportOptions || l.size()==0)
        return false;
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")
    {
        foreach(const QString &s, l)
            fprintf(hReportFile, "%s, ", s.toLatin1().data());
        fprintf(hReportFile, "\n");
    }
    else
        if (m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        fprintf(hReportFile,"<tr>\n");
        if (l.at(0).contains("<") == false && l.at(0).contains("&") == false)
            fprintf(hReportFile,"<a name=\"%s\"></a> <a href=\" \"></a>", l.at(0).toLatin1().data() );

        int i=0;
        foreach(const QString &s, l)
        {
            fprintf(hReportFile,"\t<td %s >", atts.toLatin1().data());
            if (anchors.size()>i)
                if (!anchors.at(i).isEmpty())
                    fprintf(hReportFile, "<a %s>", anchors.at(i).toLatin1().data() );
            fprintf(hReportFile, "%s", s.toLatin1().data());
            if (anchors.size()>i)
                if (!anchors.at(i).isEmpty())
                    fprintf(hReportFile, "</a>");
            fprintf(hReportFile, "</td>\n");
            i++;
        }

        fprintf(hReportFile,"</tr>\n");
    }
    return true;
}

bool CGexReport::WriteTableRow(const QList<QString> &aLineData, const QList<QString> &aLineAttributes, const QList<QString> &anchors)
{
    if (!m_pReportOptions || aLineData.size()==0)
        return false;

    QString of      = m_pReportOptions->GetOption("output", "format").toString();
    QString atts;

    if(of=="CSV")
    {
        foreach(const QString &s, aLineData)
            fprintf(hReportFile, "%s, ", s.toLatin1().data());
        fprintf(hReportFile, "\n");
    }
    else if (m_pReportOptions->isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        fprintf(hReportFile,"<tr>\n");
        if (aLineData.at(0).contains("<") == false && aLineData.at(0).contains("&") == false)
            fprintf(hReportFile,"<a name=\"%s\"></a> <a href=\" \"></a>", aLineData.at(0).toLatin1().data() );

        int i=0;
        foreach(const QString &s, aLineData)
        {
            if (aLineAttributes.size()>i && aLineAttributes.at(i).isEmpty() == false)
                atts = aLineAttributes.at(i);
            else
                atts.clear();

            fprintf(hReportFile,"\t<td %s >", atts.toLatin1().data());

            if (anchors.size()>i)
                if (!anchors.at(i).isEmpty())
                    fprintf(hReportFile, "<a %s>", anchors.at(i).toLatin1().data() );
            fprintf(hReportFile, "%s", s.toLatin1().data());
            if (anchors.size()>i)
                if (!anchors.at(i).isEmpty())
                    fprintf(hReportFile, "</a>");
            fprintf(hReportFile, "</td>\n");
            i++;
        }

        fprintf(hReportFile,"</tr>\n");
    }
    return true;
}

bool CGexReport::WriteTableRow(const HtmlTableRow& aTableLine)
{
    if (!m_pReportOptions || aTableLine.size() == 0) return false;

    QString lOuputFormat = m_pReportOptions->GetOption("output", "format").toString();

    if(lOuputFormat == "CSV")
    {
        foreach(const HtmlTableItem &lItem, aTableLine)
        {
            fprintf(hReportFile, "%s, ", lItem.mData.toLatin1().data());
        }
        fprintf(hReportFile, "\n");
    }
    else if (m_pReportOptions->isReportOutputHtmlBased())
    {
        fprintf(hReportFile,"<tr>\n");
        if (aTableLine.at(0).mData.contains("<") == false &&
                aTableLine.at(0).mData.contains("&") == false)
        {
            fprintf(hReportFile,"<a name=\"%s\"></a> <a href=\" \"></a>",
                    aTableLine.at(0).mData.toLatin1().data() );
        }

        foreach(const HtmlTableItem &lItem, aTableLine)
        {
            fprintf(hReportFile,"\t<td %s >", lItem.mAttribute.toLatin1().data());
            if (lItem.mAnchor.isEmpty() == false)
            {
                fprintf(hReportFile, "<a %s>", lItem.mAnchor.toLatin1().data());
            }
            fprintf(hReportFile, "%s", lItem.mData.toLatin1().data());
            if (lItem.mAnchor.isEmpty() == false)
            {
                fprintf(hReportFile, "</a>");
            }
            fprintf(hReportFile, "</td>\n");
        }

        fprintf(hReportFile,"</tr>\n");
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Adds field to report unless it's an empty field
/////////////////////////////////////////////////////////////////////////////
void	CGexReport::WriteInfoLine(const char *szLabel,int iData)
{
  char	szString[100];
  sprintf(szString,"%99d",iData);
  WriteInfoLine(szLabel,szString);
}

bool CGexReport::WriteText(QString txt, QString tag, QString atts)
{
    QString of=m_pReportOptions->GetOption("output", "format").toString();
    if(of=="CSV")
        fprintf(hReportFile, "%s", txt.toLatin1().data());
    else
        fprintf(hReportFile,
                "<%s %s>%s</%s>\n",
                tag.toLatin1().data(),
                atts.toLatin1().data(),
                txt.toLatin1().data(),
                tag.toLatin1().data()
                );
    return true;
}



//        int lightDark = 1;

//        float lCellSize = (float)iCell1/8;
//        for(int i = 0; i < 8; ++i)
//        {
//           QColor lColor = (lightDark%2 == 0)?lColorRef.lighter((100 + i*10)):lColorRef.darker((100 +i*10));
//           ++lightDark;
//            fprintf(hReportFile,"<td width=\"%f%%\" bgcolor=\"%s\" bordercolor=\"#000000\" style=\"table-layout:fixed;border: 0.75px solid #000000\" align=\"center\">&nbsp;</td>\n",(lCellSize), lColor.name().toLatin1().constData()) ;
//        }

//        float pecenteTotal = 0.;
//        int levelOfShade = 90/siteProportion.size();
//        //fprintf(hReportFile,"<table border=\"0\" width=\"%d%%\" cellspacing=\"0\" cellpadding=\"0\">\n", (int)iCell1);
//        fprintf(hReportFile,"<td width=\"%d%%\">&nbsp;\n", (int)iCell1);
//        for(lIterB = siteProportion.begin(); lIterB != lIterEnd; ++lIterB)
//        {
//            int siteNum = lIterB.key();
//            int valueCounter = lIterB.value();

//            //float lCellSize = ((valueCounter*(float)iCell1)/nbTotal );
//            float lCellSize = (valueCounter*100)/nbTotal;

//            //QColor lColor =lColorRef.darker((190 - levelOfShade*lightDark));

//            QColor lColor = (lightDark%2 == 0)?lColorRef.lighter((190 - levelOfShade*siteNum)):lColorRef.darker((190 - levelOfShade*siteNum));
//            ++lightDark;


//            //if(lIterB == siteProportion.begin())
//              //  fprintf(hReportFile,"<td width=\"%f%%\" bgcolor=\"%s\" bordercolor=\"#000000\" style=\"table-layout:fixed; border:0.75px solid #000000\" align=\"center\"><wbr>%d</wbr></td>\n",(lCellSize), lColor.name().toLatin1().constData(), siteNum) ;
//            //else
//               // fprintf(hReportFile,"<td width=\"%f.5%%\" bgcolor=\"%s\" bordercolor=\"#000000\" style=\"table-layout:fixed;border: 0.75px solid #000000\" align=\"center\">&nbsp;</td>\n",(lCellSize), lColor.name().toLatin1().constData()) ;
//            //if(lCellSize > 1)
//            //{
//                fprintf(hReportFile,"<td width=\"%.1f%%\" bgcolor=\"%s\" bordercolor=\"#000000\" style=\"table-layout:fixed;\" align=\"center\">&nbsp;</td>\n",(lCellSize), lColor.name().toLatin1().constData()) ;
//                pecenteTotal+=lCellSize;
//            //}
//        }
//       // fprintf(hReportFile,"</td>\n");
//        fprintf(hReportFile,"</table>");
