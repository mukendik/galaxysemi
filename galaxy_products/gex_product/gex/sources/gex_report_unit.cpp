#include <QList>
#include "gex_report_unit.h"
#include "gex_report.h"
#include "gex_constants.h"
#include "gex_shared.h"
#include "gex_group_of_files.h"
#include "gex_file_in_group.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "engine.h"

namespace GS
{
namespace Gex
{
    ReportUnit::ReportUnit(CGexReport* gr, const QString &cslkey)
        : mGexReport(gr), mKey(cslkey)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("new ReportUnit key='%1' parent='%2'...")
              .arg(cslkey)
              .arg(gr?gr->metaObject()->className():"?")
              .toLatin1().constData());
    }

    ReportUnit::~ReportUnit()
    {
        GSLOG(SYSLOG_SEV_NOTICE, "deleting a ReportUnit");
    }

    QString	ReportUnit::OpenFile(const QString& strFile)
    {
        QString strOutputFormat=mGexReport->GetOption("output", "format").toString();

        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" output format = '%1'").arg( strOutputFormat).toLatin1().constData() );
        // Open <stdf-filename>/report/advanced.htm
        if(strOutputFormat=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        {
            QString strAdvancedReportFile = strFile;

            if (strFile.isNull() || strFile.isEmpty())
                strAdvancedReportFile = mGexReport->getReportOptions()->strReportDirectory + "/pages/advanced.htm";

            mGexReport->setReportFile(NULL); //mReportFile		= NULL;
            //hAdvancedReport	= fopen(strAdvancedReportFile.toLatin1().constData(), "wt");
            mGexReport->setReportFile(fopen(strAdvancedReportFile.toLatin1().constData(), "wt"));  //mReportFile = fopen(strAdvancedReportFile.toLatin1().constData(), "wt");
        }
        else
        {
            GSLOG(SYSLOG_SEV_NOTICE, "flat HTML");
            // Creating a flat HTML file, so we do not close HTML pages between section, all pages are merged
            //hAdvancedReport = hReportFile;
        }

        if( mGexReport->getReportFile() ) //mReportFile == NULL)
            return "ok";
        else
            return "error";
    }

    void ReportUnit::BuildHeaderFooterText(QString &strHeader,QString &strFooter)
    {
       //mGexReport->BuildHeaderFooterText(strHeader, strFooter); // impossible : private !

        QString strVersionText;
        QString strProductText=" ";
        char	cSpace=' ';

        strHeader = strFooter = " ";

        // Build Version and URL info
        strVersionText = QString("Report created with: %1 - www.mentor.com")
                .arg(GS::Gex::Engine::GetInstance().Get("AppFullName").toString());

        QString of=mGexReport->GetOption("output", "format").toString();
        // Define space character (tab under word, etc...)
        if(of=="DOC"||of=="ODT")	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_WORD)
          cSpace = '\t';
        else
        if(of=="PPT")	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PPT)
          cSpace = ' ';
        else
        if(of=="PDF")	//(pReportOptions->iOutputFormat & GEX_OPTION_OUTPUT_PDF)
          cSpace = '\t';

        // Default
        strFooter = strVersionText;

        // If multiple files only display Version text as footer
        if(mGexReport->getReportOptions()->iFiles != 1)
          return;

        // Build Product & Lot ID info
        CGexGroupOfFiles *pGroup = (mGexReport->getGroupsList().isEmpty()) ? NULL : mGexReport->getGroupsList().at(0);
        if (!pGroup)
            return;
        if (pGroup->pFilesList.isEmpty())
          return;
        CGexFileInGroup *pFile  = pGroup->pFilesList.at(0);
        if(*pFile->getMirDatas().szPartType)
        {
          strProductText += "Product:" + QString(pFile->getMirDatas().szPartType).replace('\"', '_');
          strProductText += cSpace;
        }

        if(*pFile->getMirDatas().szLot)
        {
          strProductText += "  Lot:" + QString(pFile->getMirDatas().szLot).replace('\"', '_');
          strProductText += cSpace;
        }
        if(*pFile->getWaferMapData().szWaferID)
          strProductText += "  Wafer:" + QString(pFile->getWaferMapData().szWaferID).replace('\"', '_');
        else
        if(*pFile->getMirDatas().szSubLot)
          strProductText += "  SubLot:" + QString(pFile->getMirDatas().szSubLot).replace('\"', '_');

        // Finalize Header & Footer strings
        if(of=="DOC"||of=="ODT")
        {
          // Note: header line didn't look nice under Word, so now added as a second line in footer!
          strHeader = strProductText + QString("   |   Page:");
          strFooter = strVersionText;
        }
        else
        if(of=="PPT")
        {
          strHeader = " ";
          strFooter = strVersionText + QString("\t") + strProductText;
        }
        else
        if(of=="PDF")
        {
          strHeader = " ";
          strFooter = strVersionText + QString("   |   ") + strProductText;
        }
    }


    void	ReportUnit::CloseReportFile(FILE *hFile/*=NULL*/)
    {
        if(hFile != NULL)
        {
            // Close handle given in parameter...then check if it's one of the internal report handle!
            fclose(hFile);
            if (hFile == mGexReport->getReportFile()) //mReportFile)
                mGexReport->setReportFile(0); //mReportFile=0;
            if (hFile == mGexReport->getAdvancedReportFile())
                mGexReport->setAdvancedReportFile(0);
        }
        else
        {
            // just in case!
            if(mGexReport->getReportFile() != 0) //mReportFile != NULL)
            {
                fclose(mGexReport->getReportFile());
                // Check if this handle is a duplicate of the Advanced report one...
                if (mGexReport->getReportFile() == mGexReport->getAdvancedReportFile())
                    mGexReport->setAdvancedReportFile(0);
                mGexReport->setReportFile(0); //mReportFile=0;
            }
        }
    }

    void	ReportUnit::WriteHtmlSectionTitle(FILE *hReportFile, QString strBookmarkName, QString strSectionTitle)
    {
      if(mGexReport->getReportFile()==0)   //mReportFile == NULL)
            return;
      QString of=mGexReport->GetOption("output", "format").toString();
        if(of=="CSV")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_CSV)
            return;

        if(strBookmarkName.isEmpty() == false)
            fprintf(hReportFile,"<a name=\"%s\"></a>",strBookmarkName.toLatin1().constData());
        fprintf(hReportFile,"<h1 align=\"left\"><font color=\"#006699\">");
        if (of!="HTML") //if(pReportOptions->iOutputFormat != GEX_OPTION_OUTPUT_HTML)
        {
            // Flat HTML file: Add 'Quantix' button icon before each Section title as hyperlink to Quantix's Web site
            fprintf(hReportFile,"<a href=\"http://www.mentor.com\"><img border=\"0\" src=\"../images/quantix.png\"></a>&nbsp;");
        }

        // Write H1 Title in HTML page...or Examinator Web site if PowerPoint slide (as Slide Title is defined few ines beow with 'SetPowerPointSlideName')
        QString strH1Title = strSectionTitle;
        if(of=="PPT")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_PPT)
        {
            switch(GS::LPPlugin::ProductInfo::getInstance()->getProductID())
            {
//                case GEX_DATATYPE_GEX_YIELD123:		// Yield123
//                    strH1Title = "Yield123";
//                    break;

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

        if(of=="HTML")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_HTML)
        {
            fprintf(hReportFile,"<img border=\"0\" src=\"../images/ruler.png\" width=\"616\" height=\"8\">\n");
            fprintf(hReportFile,"<br>\n");
        }
        // If creating a Powerpoint presentation, save Section title.
        if(of=="PPT")	//(pReportOptions->iOutputFormat == GEX_OPTION_OUTPUT_PPT)
            mGexReport->SetPowerPointSlideName(strSectionTitle);
    }

} // namespace Gex
} // namespace GS
